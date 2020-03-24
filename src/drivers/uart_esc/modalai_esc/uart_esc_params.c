/****************************************************************************
 *
 *   Copyright (c) 2020 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <px4_config.h>
#include <parameters/param.h>

/**
 * UART ESC config
 *
 * @value 0 - None
 * @value 1 - ModalAI
 *
 * @group UART ESC
 */
PARAM_DEFINE_INT32(UART_ESC_CONFIG, 0);

/**
 * UART ESC baud rate
 *
 * Default rate is 250Kbps, which is used in off-the-shelf MoadalAI ESC products.
 *
 * @group UART ESC
 */
PARAM_DEFINE_INT32(UART_ESC_BAUD, 250000);

/**
 * UART ESC Motor 1 Mapping.  1-4 (negative for reversal).
 *
 * @group UART ESC
 */
PARAM_DEFINE_INT32(UART_ESC_MOTOR1, 3);

/**
 *UART ESC Motor 2 Mapping.  1-4 (negative for reversal).
 *
 * @group UART ESC
 */
PARAM_DEFINE_INT32(UART_ESC_MOTOR2, 2);

/**
 * UART ESC Motor 3 Mapping.  1-4 (negative for reversal).
 *
 * @group UART ESC
 */
PARAM_DEFINE_INT32(UART_ESC_MOTOR3, 4);

/**
 * UART ESC Motor 4 Mapping.  1-4 (negative for reversal).
 *
 * @group UART ESC
 */
PARAM_DEFINE_INT32(UART_ESC_MOTOR4, 1);

/**
 * UART ESC RPM Min
 *
 * Minimum RPM for ESC
 *
 * @group UART ESC
 */
PARAM_DEFINE_INT32(UART_ESC_RPM_MIN, 0);

/**
 * UART ESC RPM Max
 *
 * Maximum RPM for ESC
 *
 * @group UART ESC
 */
PARAM_DEFINE_INT32(UART_ESC_RPM_MAX, 18500);
