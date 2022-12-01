/****************************************************************************
 *
 *   Copyright (c) 2016 PX4 Development Team. All rights reserved.
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

/**
 * @file qshell.cpp
 * Listener for shell commands from posix
 *
 * @author Nicolas de Palezieux <ndepal@gmail.com>
 */

#include "qshell.h"

#include <px4_platform_common/log.h>
#include <px4_platform_common/time.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/defines.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <uORB/topics/qshell_retval.h>
#include <drivers/drv_hrt.h>

#define MAX_ARGS 8 // max number of whitespace separated args after app name

px4::AppState QShell::appState;

QShell::QShell()
{
	PX4_INFO("Init app map initialized");
	init_app_map(m_apps);
}

int QShell::main()
{
	appState.setRunning(true);
	PX4_INFO("app state set to true");

	int sub_qshell_req = orb_subscribe(ORB_ID(qshell_req));

	if (sub_qshell_req == PX4_ERROR) {
		PX4_ERR("Error subscribing to qshell_req topic");
		return -1;
	}

	PX4_INFO("subscription to qshell req made");

	px4_pollfd_struct_t fds[1] = {};
	fds[0].fd = sub_qshell_req;
	fds[0].events = POLLIN;
	PX4_INFO("started polling");

	while (!appState.exitRequested()) {

		int pret = px4_poll(&fds[0], (sizeof(fds) / sizeof(fds[0])), 1000);

		if (pret > 0 && fds[0].revents & POLLIN) {

			orb_copy(ORB_ID(qshell_req), sub_qshell_req, &m_qshell_req);

			PX4_INFO("qshell gotten: %s", m_qshell_req.cmd);

			char current_char;
			std::string arg;
			std::vector<std::string> appargs;
			PX4_INFO("variables created for arg current char and appargs");

			for (unsigned str_idx = 0; str_idx < m_qshell_req.strlen; str_idx++) {
				current_char = m_qshell_req.cmd[str_idx];

				if (isspace(current_char)) { // split at spaces
					if (arg.length()) {
						appargs.push_back(arg);
						arg = "";
						PX4_INFO("appargs pushed back from isspace fucntion inside forloop");

					}

				} else {
					arg += current_char;
				}
			}

			appargs.push_back(arg);  // push last argument
			PX4_INFO("Appargs pushed back outside forloop");

			qshell_retval_s retval{};
			PX4_INFO("qshell retval created ");

			retval.return_value = run_cmd(appargs);
			PX4_INFO("run_cmd ran");

			retval.return_sequence = m_qshell_req.request_sequence;
			PX4_INFO("VALUE OR RETURN VALUE: %i", retval.return_value);

			if (retval.return_value) {
				PX4_ERR("Failed to execute command: %s", m_qshell_req.cmd);

			} else {
				PX4_INFO("Ok executing command: %s", m_qshell_req.cmd);
			}

			retval.timestamp = hrt_absolute_time();
			PX4_INFO("Sending qshell retval with timestamp %llu, current timestamp %llu", retval.timestamp, hrt_absolute_time());
			_qshell_retval_pub.publish(retval);
			PX4_INFO("RETVAL FROM SLPI QSHELL PUBLISHED");

		} else if (pret == 0) {

			// Timing out is fine.
		} else {
			// Something is wrong.
			usleep(10000);
		}
	}

	PX4_INFO("app state to set running to true still");

	appState.setRunning(false);
	PX4_INFO("app state to set running false");

	return 0;
}

int QShell::run_cmd(const std::vector<std::string> &appargs)
{
	// command is appargs[0]
	PX4_INFO("setting command to appargs 0");
	std::string command = appargs[0];

	if (command.compare("help") == 0) {
		list_builtins(m_apps);
		return 0;
	}

	//replaces app.find with iterator code to avoid null pointer exception
	for (apps_map_type::iterator it = m_apps.begin(); it != m_apps.end(); ++it) {
		if (it->first == command) {
			// one for command name, one for null terminator
			const char *arg[MAX_ARGS + 2];

			unsigned int i = 0;

			if (appargs.size() > MAX_ARGS + 1) {
				PX4_ERR("%d too many arguments in run_cmd", appargs.size() - (MAX_ARGS + 1));
				return 1;
			}

			while (i < appargs.size() && appargs[i].c_str()[0] != '\0') {
				arg[i] = (char *)appargs[i].c_str();
				PX4_INFO("  arg%d = '%s'\n", i, arg[i]);
				++i;
			}

			arg[i] = (char *)0;

			//PX4_DEBUG_PRINTF(i);
			if (m_apps[command] == NULL) {
				PX4_ERR("Null function !!\n");

			} else {
				PX4_INFO("run cmd ran the right way!!!");
				int x = m_apps[command](i, (char **)arg);
				PX4_INFO("Ran m_apps command just fine");
				return x;
			}

		}
	}

	PX4_ERR("Command %s not found", command.c_str());
	return 1;
}
