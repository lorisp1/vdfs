#! /bin/bash

ls_mt ()
{
	if [[ $# -eq 1 ]] || [[ $# -eq 2 ]]; then
	{
		if [[ ${1:0:3} == "VD:" ]] || [[ ${2:0:3} == "VD:" ]]; then
			vd_client ls $@
		else
			command ls $@
		fi
	}
	else
		command ls $@
	fi
}

cp ()
{
	if [[ $# -eq 2 ]]; then
	{
		if [[ ${1:0:3} == "VD:" ]]; then
			vd_client cp $@
		else
			if [[ ${2:0:3} == "VD:" ]]; then
				vd_client cp $@
			else
				command cp $@
			fi
		fi
	}
	else
		command cp $@
	fi
}

mkdir ()
{
	if [[ $# -eq 1 ]]; then
	{
		if [[ ${1:0:3} == "VD:" ]]; then
			vd_client mkdir $@
		else
			command mkdir $@
		fi
	}
	else
		command mkdir $@
	fi
}

del ()
{
	vd_client del $@
}

undel ()
{
	vd_client undel $@
}

purge ()
{
	vd_client purge $@
}

rm ()
{
	if [[ $# -eq 1 ]]; then
	{
		if [[ ${1:0:3} == "VD:" ]]; then
			vd_client rm $@
		else
			command rm $@
		fi
	}
	else
		command rm $@
	fi
}

ln ()
{
	if [[ $# -eq 2 ]] || [[ $# -eq 3 ]]; then
	{
		if [[ ${1:0:3} == "VD:" ]]; then
			vd_client ln $@
		else
		{
			if [[ ${2:0:3} == "VD:" ]]; then
				vd_client ln $@
			else
			{
				if [[ ${3:0:3} == "VD:" ]]; then
					vd_client ln $@
				else
					command ln $@
				fi
			}
			fi
		}
		fi
	}
	else
		command ln $@
	fi
}
