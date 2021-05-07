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

#include "string.h"
#include "modalai_esc_serial.hpp"

#ifdef __PX4_QURT
void configure_uart_callbacks(open_uart_func_t open_func,
                              write_uart_func_t write_func,
                              read_uart_func_t read_func) {
    ModalaiEscSerial::configure_callbacks(open_func, write_func, read_func);
}

// Static variables
bool ModalaiEscSerial::_callbacks_configured = false;
open_uart_func_t  ModalaiEscSerial::_open_uart = NULL;
write_uart_func_t ModalaiEscSerial::_write_uart = NULL;
read_uart_func_t  ModalaiEscSerial::_read_uart = NULL;
#endif

ModalaiEscSerial::ModalaiEscSerial()
{
}

ModalaiEscSerial::~ModalaiEscSerial()
{
	if (_uart_fd >= 0) {
		uart_close();
	}
}

int ModalaiEscSerial::uart_open(const char *dev, speed_t speed)
{
	if (_uart_fd >= 0) {
		PX4_ERR("Port in use: %s (%i)", dev, errno);
		return -1;
	}

#ifdef __PX4_QURT
    if (_callbacks_configured) {
        // Convert device string into a uart port number
        char *endptr = NULL;
        uint8_t port_number = (uint8_t) strtol(dev, &endptr, 10);
        if ((port_number == 0) && (endptr == dev)) {
            PX4_ERR("Could not convert %s into a valid uart port number", dev);
            return -1;
        }
    	_uart_fd = _open_uart(port_number, speed);
    } else {
        PX4_ERR("Cannot open uart until callbacks have been configured");
        return -1;
    }
#else
	/* Open UART */
	_uart_fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
#endif

	if (_uart_fd < 0) {
		PX4_ERR("Error opening port: %s (%i)", dev, errno);
		return -1;
	}

#ifndef __PX4_QURT
	/* Back up the original UART configuration to restore it after exit */
	int termios_state;

	if ((termios_state = tcgetattr(_uart_fd, &_orig_cfg)) < 0) {
		PX4_ERR("Error configuring port: tcgetattr %s: %d", dev, termios_state);
		uart_close();
		return -1;
	}

	/* Fill the struct for the new configuration */
	tcgetattr(_uart_fd, &_cfg);

	/* Disable output post-processing */
	_cfg.c_oflag &= ~OPOST;

	_cfg.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	_cfg.c_cflag &= ~CSIZE;
	_cfg.c_cflag |= CS8;                 /* 8-bit characters */
	_cfg.c_cflag &= ~PARENB;             /* no parity bit */
	_cfg.c_cflag &= ~CSTOPB;             /* only need 1 stop bit */
	_cfg.c_cflag &= ~CRTSCTS;            /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	_cfg.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	_cfg.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

	if (cfsetispeed(&_cfg, speed) < 0 || cfsetospeed(&_cfg, speed) < 0) {
		PX4_ERR("Error configuring port: %s: %d (cfsetispeed, cfsetospeed)", dev, termios_state);
		uart_close();
		return -1;
	}

	if ((termios_state = tcsetattr(_uart_fd, TCSANOW, &_cfg)) < 0) {
		PX4_ERR("Error configuring port: %s (tcsetattr)", dev);
		uart_close();
		return -1;
	}
#endif

	return 0;
}

int ModalaiEscSerial::uart_close()
{
#ifndef __PX4_QURT
	if (_uart_fd < 0) {
		PX4_ERR("invalid state for closing");
		return -1;
	}

	if (tcsetattr(_uart_fd, TCSANOW, &_orig_cfg)) {
		PX4_ERR("failed restoring uart to original state");
	}

	if (close(_uart_fd)) {
		PX4_ERR("error closing uart");
	}
#endif

	_uart_fd = -1;

	return 0;
}

int ModalaiEscSerial::uart_write(FAR void *buf, size_t len)
{
	if (_uart_fd < 0 || buf == NULL) {
		PX4_ERR("invalid state for writing or buffer");
		return -1;
	}

#ifdef __PX4_QURT
    if (_callbacks_configured) {
        return _write_uart(_uart_fd, buf, len);
    } else {
        PX4_ERR("Cannot write to uart until callbacks have been configured");
        return -1;
    }
#else
	return write(_uart_fd, buf, len);
#endif
}

int ModalaiEscSerial::uart_read(FAR void *buf, size_t len)
{
	if (_uart_fd < 0 || buf == NULL) {
		PX4_ERR("invalid state for reading or buffer");
		return -1;
	}

#ifdef __PX4_QURT
    if (_callbacks_configured) {
        return _read_uart(_uart_fd, buf, len);
    } else {
        PX4_ERR("Cannot read from uart until callbacks have been configured");
        return -1;
    }
#else
	return read(_uart_fd, buf, len);
#endif
}
