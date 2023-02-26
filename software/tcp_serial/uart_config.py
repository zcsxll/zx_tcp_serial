from tracemalloc import stop


uart_config = { #from esp32-hal-uart.h
    '5N1'  : 0x8000010,
    '6N1'  : 0x8000014,
    '7N1'  : 0x8000018,
    '8N1'  : 0x800001c,
    '5N2'  : 0x8000030,
    '6N2'  : 0x8000034,
    '7N2'  : 0x8000038,
    '8N2'  : 0x800003c,
    '5E1'  : 0x8000012,
    '6E1'  : 0x8000016,
    '7E1'  : 0x800001a,
    '8E1'  : 0x800001e,
    '5E2'  : 0x8000032,
    '6E2'  : 0x8000036,
    '7E2'  : 0x800003a,
    '8E2'  : 0x800003e,
    '5O1'  : 0x8000013,
    '6O1'  : 0x8000017,
    '7O1'  : 0x800001b,
    '8O1'  : 0x800001f,
    '5O2'  : 0x8000033,
    '6O2'  : 0x8000037,
    '7O2'  : 0x800003b,
    '8O2'  : 0x800003f
}

def get_uart_config(data_bit, check_bit, stop_bit):
    key = f'{data_bit}{check_bit}{stop_bit}'
    return uart_config[key]