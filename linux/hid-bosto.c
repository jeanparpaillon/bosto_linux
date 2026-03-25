#include <linux/module.h>
#include <linux/hid.h>
#include <linux/input.h>

#define USB_VENDOR_ID_BOSTO  0x0ED1
#define USB_PRODUCT_ID_BOSTO 0x7821

#define REPORT_PEN 0x01
#define REPORT_AUX 0x04

/* Raw coordinate space */
#define MAX_X_RAW 17920
#define MAX_Y_RAW 10240

/* Calibration: stretch (fixed-point x1000) and offset */
#define X_STRETCH 3250  /* 3.25 * 1000 */
#define Y_STRETCH 3115  /* 3.115 * 1000 */
#define X_OFFSET  20300
#define Y_OFFSET  11200

#define CX (MAX_X_RAW / 2)  /* 8960 */
#define CY (MAX_Y_RAW / 2)  /* 5120 */

/* Physical dimensions */
#define WIDTH_MM  290
#define HEIGHT_MM 170
#define LPMM      200  /* lines per mm */
#define MAX_X     (WIDTH_MM * LPMM)   /* 58000 */
#define MAX_Y     (HEIGHT_MM * LPMM)  /* 34000 */

#define MAX_PRESSURE 8191
#define AUX_BTN_COUNT 8

struct bosto {
	struct input_dev *pen_input;
	struct input_dev *pad_input;
};

static inline int correct_x(u16 x_raw)
{
	return (int)((s32)(x_raw - CX) * X_STRETCH / 1000) + CX + X_OFFSET;
}

static inline int correct_y(u16 y_raw)
{
	return (int)((s32)(y_raw - CY) * Y_STRETCH / 1000) + CY + Y_OFFSET;
}

static void handle_pen(struct input_dev *input, u8 *data)
{
	u8 status = data[1];

	bool prox = status & (1 << 4);
	bool tip  = status & (1 << 0);
	bool b1   = status & (1 << 5);
	bool b2   = status & (1 << 1);

	u16 x_raw = data[2] | (data[3] << 8);
	u16 y_raw = data[4] | (data[5] << 8);
	u16 pressure = data[6] | (data[7] << 8);

	int x = correct_x(x_raw);
	int y = correct_y(y_raw);

	input_report_key(input, BTN_TOOL_PEN, prox);
	input_report_key(input, BTN_TOUCH, tip);
	input_report_key(input, BTN_STYLUS, b1);
	input_report_key(input, BTN_STYLUS2, b2);

	if (prox) {
		input_report_abs(input, ABS_X, x);
		input_report_abs(input, ABS_Y, y);
		input_report_abs(input, ABS_PRESSURE, tip ? pressure : 0);
	} else {
		input_report_abs(input, ABS_PRESSURE, 0);
	}

	input_sync(input);
}

static void handle_aux(struct input_dev *input, u8 *data)
{
	u8 aux = data[4];
	int i;

	for (i = 0; i < AUX_BTN_COUNT; i++)
		input_report_key(input, BTN_0 + i, (aux >> i) & 1);

	input_sync(input);
}

static int bosto_raw_event(struct hid_device *hdev,
                           struct hid_report *report,
                           u8 *data, int size)
{
	struct bosto *b = hid_get_drvdata(hdev);

	pr_info("Raw event: report ID %d, size %d\n", data[0], size);
	
	if (size < 8)
		return 0;

	if (data[0] == REPORT_PEN)
		handle_pen(b->pen_input, data);
	else if (data[0] == REPORT_AUX)
		handle_aux(b->pad_input, data);
	else
		return 0;

	return 1;
}

static struct input_dev *bosto_alloc_pen(struct hid_device *hdev)
{
	struct input_dev *input;

	input = devm_input_allocate_device(&hdev->dev);
	if (!input)
		return NULL;

	input->name = "BOSTO 13HD Pen";
	input->phys = hdev->phys;
	input->uniq = hdev->uniq;
	input->id.bustype = BUS_USB;
	input->id.vendor  = hdev->vendor;
	input->id.product = hdev->product;

	input_set_capability(input, EV_KEY, BTN_TOOL_PEN);
	input_set_capability(input, EV_KEY, BTN_TOUCH);
	input_set_capability(input, EV_KEY, BTN_STYLUS);
	input_set_capability(input, EV_KEY, BTN_STYLUS2);

	input_set_abs_params(input, ABS_X, 0, MAX_X, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, MAX_Y, 0, 0);
	input_set_abs_params(input, ABS_PRESSURE, 0, MAX_PRESSURE, 0, 0);

	input_abs_set_res(input, ABS_X, LPMM);
	input_abs_set_res(input, ABS_Y, LPMM);

	__set_bit(INPUT_PROP_DIRECT, input->propbit);

	return input;
}

static struct input_dev *bosto_alloc_pad(struct hid_device *hdev)
{
	struct input_dev *input;
	int i;

	input = devm_input_allocate_device(&hdev->dev);
	if (!input)
		return NULL;

	input->name = "BOSTO 13HD Pad";
	input->phys = hdev->phys;
	input->uniq = hdev->uniq;
	input->id.bustype = BUS_USB;
	input->id.vendor  = hdev->vendor;
	input->id.product = hdev->product;

	for (i = 0; i < AUX_BTN_COUNT; i++)
		input_set_capability(input, EV_KEY, BTN_0 + i);

	return input;
}

static int bosto_probe(struct hid_device *hdev,
                       const struct hid_device_id *id)
{
	struct bosto *b;
	int ret;

	b = devm_kzalloc(&hdev->dev, sizeof(*b), GFP_KERNEL);
	if (!b)
		return -ENOMEM;

	hid_set_drvdata(hdev, b);

	ret = hid_parse(hdev);
	if (ret)
		return ret;

	ret = hid_hw_start(hdev, HID_CONNECT_HIDRAW);
	if (ret)
		return ret;

	ret = hid_hw_open(hdev);
	if (ret)
		return ret;

	b->pen_input = bosto_alloc_pen(hdev);
	if (!b->pen_input)
		return -ENOMEM;

	ret = input_register_device(b->pen_input);
	if (ret)
		return ret;

	b->pad_input = bosto_alloc_pad(hdev);
	if (!b->pad_input)
		return -ENOMEM;

	ret = input_register_device(b->pad_input);
	if (ret)
		return ret;

	return 0;
}

static const struct hid_device_id bosto_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_BOSTO, USB_PRODUCT_ID_BOSTO) },
	{}
};
MODULE_DEVICE_TABLE(hid, bosto_devices);

static void bosto_remove(struct hid_device *hdev)
{
	hid_hw_close(hdev);
	hid_hw_stop(hdev);
}

static struct hid_driver bosto_driver = {
	.name = "hid-bosto",
	.id_table = bosto_devices,
	.probe = bosto_probe,
	.remove = bosto_remove,
	.raw_event = bosto_raw_event,
};

module_hid_driver(bosto_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BOSTO Tablet Driver");
MODULE_AUTHOR("Jean Parpaillon");
MODULE_VERSION(MODULE_VER);
