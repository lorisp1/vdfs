#destination directory
prefix=${HOME}
root_prefix=/usr
vddir=${prefix}/.vd
bindir=${vddir}/bin
server_log_folder=/tmp/vd_server_logs

all: 
	make -C vd_server all
	make -C vd_client all

install:
	if ! { cp vd_server/vd_server ${root_prefix}/bin/ 2> /dev/null && cp vd_client/vd_client ${root_prefix}/bin/ 2> /dev/null && cp vd ${root_prefix}/bin/ 2> /dev/null && cp vd.fun /opt 2> /dev/null; }; then\
	{\
		mkdir ${vddir};\
		mkdir ${bindir};\
		mkdir ${server_log_folder};\
		cp vd_server/vd_server ${bindir};\
		cp vd_client/vd_client ${bindir};\
		cp vd ${bindir};\
		cp vd.fun ${vddir};\
		touch ${HOME}/.profile;\
		echo -n "PATH=$$" | cat ${HOME}/.profile - | cat > ${HOME}/.profile;\
		echo "PATH:${bindir}/" | cat ${HOME}/.profile - | cat > ${HOME}/.profile;\
		echo "source ${vddir}/vd.fun" | cat ${HOME}/.profile - | cat > ${HOME}/.profile;\
		echo "alias ls='ls_mt'" | cat ${HOME}/.profile - | cat > ${HOME}/.profile;\
		exec bash -l;\
	}\
	else\
	{\
		mkdir ${server_log_folder};\
		echo "source ${vddir}/vd.fun" | cat /etc/profile - | cat > /etc/profile;\
		echo "alias ls='ls_mt'" | cat /etc/profile - | cat > /etc/profile;\
		exec bash -l;\
	}\
	fi;\
	

clean:
	make -C vd_server clean
	make -C vd_client clean
	rm -f vd_server/vd_server
	rm -f vd_client/vd_client

uninstall:
	if ! { rm ${root_prefix}/bin/vd_server 2> /dev/null && rm ${root_prefix}/bin/vd_client 2> /dev/null && rm ${root_prefix}/bin/vd && rm /opt/vd.fun; }; then\
	{\
		more ${HOME}/.profile | grep -v "PATH:${bindir}/" > ${HOME}/.profile;\
		more ${HOME}/.profile | grep -v "source ${vddir}/vd.fun" > ${HOME}/.profile;\
		more ${HOME}/.profile | grep -v "alias ls='ls_mt'" > ${HOME}/.profile;\
		rm -rf ${server_log_folder};\
		rm -rf ${vddir};\
		exec bash -l;\
	}\
	else\
	{\
		more /etc/profile | grep -v "source ${vddir}/vd.fun" > /etc/profile;\
		more /etc/profile | grep -v "alias ls='ls_mt'" > /etc/profile;\
		rm -rf ${server_log_folder};\
		exec bash -l;\
	}\
	fi;
