
#include "ask.h"

#include <string.h>

#include "ask_config.h"

//################################################################################################################
bool ask_init(ask_t *ask)
{
    ask->lock = false;
    if (ask->fn_micros == NULL)
        return false;
    if (ask->fn_delay_ms == NULL)
        return false;
    ask->detect_busy = false;
    if (ask->fn_init_rx != NULL)
        ask->fn_init_rx();
    if (ask->fn_read_pin != NULL)
        ask->enable_rx = true;
    if (ask->fn_init_tx != NULL)
        ask->fn_init_tx();
    if (ask->fn_write_pin != NULL) {
        if (ask->fn_delay_us == NULL)
            return false;
        ask->fn_write_pin(false);
    }
    return true;
}
//################################################################################################################
#include "hal_gpio.h"
void ask_pinchange_callback(ask_t *ask)
{
    if (!ask->enable_rx)
        return;
    if (!ask->detect_end) {
        if (!ask->detect_begin) {
            if (ask->fn_read_pin() == 0) {
                ask->fn_restat_tm();
                ask->detect_begin = true;
                ask->detect_sync = 0;
                ask->index = 0;
            }
        } else {
            if (ask->detect_sync == 0) {
                if (ask->fn_read_pin()) {  //低电平时间
                    ask->buffer[0] = ask->fn_micros() - ask->time;
                    if (ask->buffer[0] > _ASK_SYNC_HEAD_LEN_ * (100 - _ASK_TOLERANCE_) / 100 &&
                        ask->buffer[0] < _ASK_SYNC_HEAD_LEN_ * (100 + _ASK_TOLERANCE_) / 100) {
                        ask->detect_sync = 1;
                        ask->index = 0;
                    }
                } else {
                    ask->buffer[0] = ask->fn_micros() - ask->time;
                    if (ask->buffer[0] > _ASK_SYNC_HEAD_LEN_ * 2)
                        ask->detect_end = true;
                }
            } else {
                // if(ask->fn_read_pin()){//低电平时间
                // }else{
                // }
                ask->buffer[ask->index] = ask->fn_micros() - ask->time;
                if (ask->index < (_ASK_MAX_BYTE_LEN_ * 16))
                    ask->index++;
                else
                    ask->detect_end = true;
            }
        }
    }
    ask->time = ask->fn_micros();
}
//################################################################################################################
bool ask_available(ask_t *ask)
{
    if (ask->lock == true)
        return false;
    ask->lock = true;
    // if (ask->index > (_ASK_MAX_BYTE_LEN_ * 16 + 1))
    // {
    //     ask->detect_begin = false;
    //     ask->detect_end = false;
    //     ask->lock = false;
    //     return false;
    // }

    if (ask->detect_end && ask->detect_sync && !ask->detect_busy) {
        do {
            if (ask->index < (_ASK_MIN_BYTE_LEN_ * 16))
                break;  //{    ask->buffer[0] = 0; break;}
            memset(ask->buffer_byte, 0, _ASK_MAX_BYTE_LEN_);
            ask->buffer_time = ask->buffer[0] + ask->buffer[1];
            if (ask->buffer[0] > ask->buffer[1]) {
                ask->buffer_time_high[0] =
                    ask->buffer[0] - ((_ASK_TOLERANCE_ * ask->buffer[0]) / 100);
                ask->buffer_time_high[1] =
                    ask->buffer[0] + ((_ASK_TOLERANCE_ * ask->buffer[0]) / 100);
                ask->buffer_time_low[0] =
                    ask->buffer[1] - ((_ASK_TOLERANCE_ * ask->buffer[1]) / 100);
                ask->buffer_time_low[1] =
                    ask->buffer[1] + ((_ASK_TOLERANCE_ * ask->buffer[1]) / 100);
            } else if (ask->buffer[0] < ask->buffer[1]) {
                ask->buffer_time_high[0] =
                    ask->buffer[1] - ((_ASK_TOLERANCE_ * ask->buffer[1]) / 100);
                ask->buffer_time_high[1] =
                    ask->buffer[1] + ((_ASK_TOLERANCE_ * ask->buffer[1]) / 100);
                ask->buffer_time_low[0] =
                    ask->buffer[0] - ((_ASK_TOLERANCE_ * ask->buffer[0]) / 100);
                ask->buffer_time_low[1] =
                    ask->buffer[0] + ((_ASK_TOLERANCE_ * ask->buffer[0]) / 100);
            } else
                break;  //{    ask->buffer[0] = 1; break;}
            ask->data_bit = 0;
            memset(ask->data_byte, 0, _ASK_MAX_BYTE_LEN_);
            for (uint16_t i = 0; i < ask->index; i += 2) {
                //  detect 0 data
                if ((ask->buffer[i] > ask->buffer_time_low[0]) &&
                    (ask->buffer[i] < ask->buffer_time_low[1]) &&
                    (ask->buffer[i + 1] > ask->buffer_time_high[0]) &&
                    (ask->buffer[i + 1] < ask->buffer_time_high[1])) {
                    ask->data_bit++;
                }
                //  detect 1 data
                else if ((ask->buffer[i + 1] > ask->buffer_time_low[0]) &&
                         (ask->buffer[i + 1] < ask->buffer_time_low[1]) &&
                         (ask->buffer[i] > ask->buffer_time_high[0]) &&
                         (ask->buffer[i] < ask->buffer_time_high[1])) {
                    ask->data_byte[ask->data_bit / 8] |= 0x80 >> (ask->data_bit % 8);
                    ask->data_bit++;
                } else
                    break;  //{    ask->buffer[i] = 2; break;}
            }
            if (ask->data_bit % 8 != 0)
                break;
            if ((ask->data_bit > (_ASK_MAX_BYTE_LEN_ * 8)) ||
                (ask->data_bit < (_ASK_MIN_BYTE_LEN_ * 8)))
                break;
            ask->detect_busy = true;
            ask->lock = false;
            return true;

        } while (0);

        ask->detect_begin = false;
        ask->detect_end = false;
        ask->lock = false;
        return false;
    } else if (ask->detect_busy) {
        ask->lock = false;
        return true;
    }
    if (ask->detect_end && ask->detect_begin) {
        ask->detect_begin = false;
        ask->detect_end = false;
        ask->lock = false;
        return false;
    }
    ask->lock = false;
    return false;
}
//################################################################################################################
void ask_wait(ask_t *ask)
{
    while (1) {
        ask_reset_available(ask);
        ask->fn_delay_ms(200);
        if (ask_available(ask) == false)
            break;
    }
}
//################################################################################################################
void ask_reset_available(ask_t *ask)
{
    while (ask->lock == true) ask->fn_delay_ms(1);
    ask->lock = true;
    memset(ask->buffer, 0, sizeof(ask->buffer));
    ask->detect_begin = false;
    ask->detect_end = false;
    ask->detect_busy = false;
    ask->lock = false;
}
//################################################################################################################
uint8_t ask_read_bytes(ask_t *ask, uint8_t *data)
{
    while (ask->lock == true) ask->fn_delay_ms(1);
    ask->lock = true;
    memcpy(data, ask->data_byte, ask->data_bit / 8);
    ask->lock = false;

    ask_reset_available(ask);
    return ask->data_bit / 8;
}
//################################################################################################################
uint16_t ask_read_time_of_bit(ask_t *ask) { return ask->buffer_time; }
//################################################################################################################
void ask_send_bytes(ask_t *ask, uint8_t *data, uint8_t len, uint32_t bit_time_us,
                    uint8_t try_to_send)
{
    uint32_t lck = bit_time_us / 16;
    if (ask->fn_write_pin == NULL)
        return;
    while (ask->lock == true) ask->fn_delay_ms(1);
    ask->lock = true;
    if (ask->fn_read_pin != NULL)
        ask->enable_rx = false;
    // ask->fn_write_pin(true);
    // ask->fn_delay_ms(bit_time_us * 8 / 1000);
    for (uint8_t t = 0; t < try_to_send; t++) {
        ask->fn_write_pin(true);
        // ask->fn_delay_us(lck * 4 );
        ask->fn_delay_us(350);
        ask->fn_write_pin(false);
        // ask->fn_delay_us(lck * 124 );
        ask->fn_delay_us(350 * 31);
        for (uint8_t byte = 0; byte < len; byte++) {
            for (int8_t bit = 7; bit > -1; bit--) {
                if (data[byte] & (1 << bit)) {
                    ask->fn_write_pin(true);
                    // ask->fn_delay_us(lck * 12 );
                    ask->fn_delay_us(350 * 3);
                    ask->fn_write_pin(false);
                    // ask->fn_delay_us(lck * 4 );
                    ask->fn_delay_us(350);
                } else {
                    ask->fn_write_pin(true);
                    // ask->fn_delay_us(lck * 4 );
                    ask->fn_delay_us(350);
                    ask->fn_write_pin(false);
                    // ask->fn_delay_us(lck * 12 );
                    ask->fn_delay_us(350 * 3);
                }
            }
        }
        // ask->fn_write_pin(true);
        // ask->fn_delay_us(bit_time_us * 25 / 100);
        // ask->fn_write_pin(false);
    }
    if (ask->fn_read_pin != NULL)
        ask->enable_rx = true;
    ask->lock = false;
}
//################################################################################################################
int16_t ask_checkChannelLast4Bit(uint8_t *newCode, uint8_t *refrence, uint8_t len)
{
    if (len < 1)
        return -1;
    uint8_t maskNew[len];
    uint8_t maskRef[len];
    memcpy(maskNew, newCode, len);
    memcpy(maskRef, refrence, len);
    maskNew[len - 1] &= 0xF0;
    maskRef[len - 1] &= 0xF0;
    if (memcmp(maskNew, maskRef, len) != 0)
        return -1;
    return newCode[len - 1] & 0x0F;
}
//################################################################################################################
int16_t ask_checkChannelLast8Bit(uint8_t *newCode, uint8_t *refrence, uint8_t len)
{
    if (len < 1)
        return -1;
    if (memcmp(newCode, refrence, len - 1) != 0)
        return -1;
    return newCode[len - 1];
}
//################################################################################################################
