#!/usr/bin/env bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR/jMAVSim"

udp_port=4560
extra_args=
baudrate=921600
device=
ip="127.0.0.1"
while getopts ":b:d:p:qsr:f:i:lo" opt; do
	case $opt in
		b)
			baudrate=$OPTARG
			;;
		d)
			device="$OPTARG"
			;;
		i)
			ip="$OPTARG"
			;;
		p)
			udp_port=$OPTARG
			;;
		q)
			extra_args="$extra_args -qgc"
			;;
		s)
			extra_args="$extra_args -sdk"
			;;
		r)
			extra_args="$extra_args -r $OPTARG"
			;;
		l)
			extra_args="$extra_args -lockstep"
			;;
		o)
			extra_args="$extra_args -disponly"
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			exit 1
			;;
	esac
done

if [ "$device" == "" ]; then
	device="-udp $ip:$udp_port"
elif [ "$device" == "/dev/*" ]; then
	device="-serial $device $baudrate";
else
	device="-serialudp $device $baudrate $ip:$udp_port";
fi


if [ "$HEADLESS" = "1" ]; then
    extra_args="$extra_args -no-gui"
fi

ant create_run_jar copy_res
cd out/production
echo $device $extra_args
java -XX:GCTimeRatio=20 -Djava.ext.dirs= -jar jmavsim_run.jar $device $extra_args
