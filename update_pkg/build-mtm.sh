#!/bin/bash

### made by gandy

BIN_NAME=mts_main
PKG_NAME=update.tar.gz
ROOT_DIR=$(pwd)

EZV_BUILD_DIR=$(pwd)/../EZV/src
EZV_BIN_DIR=$(pwd)/../EZV/bin

HDT_BUILD_DIR=$(pwd)/../HDT/src
HDT_BIN_DIR=$(pwd)/../HDT/bin

KCM_BUILD_DIR=$(pwd)/../KOCOM/src
KCM_BIN_DIR=$(pwd)/../KOCOM/bin

SHN_BUILD_DIR=$(pwd)/../SAMSUNG/src
SHN_BIN_DIR=$(pwd)/../SAMSUNG/bin

function build_ezv()
{
	echo "==============================================="
	echo "  *** build EZV applicatin ***  "
	echo "==============================================="

	cd ${EZV_BUILD_DIR}
	
	make clean
	make all
	rc=$?
	if [ ${rc} != 0 ] ; then
		echo "Build error(code:${rc})"
		exit ${rc}
	fi
	
	cd ${ROOT_DIR}
}

function build_hdt()
{
	echo "==============================================="
	echo "  *** build HDT applicatin ***  "
	echo "==============================================="

	cd ${HDT_BUILD_DIR}
	
	make clean
	make all

	cd ${ROOT_DIR}
}

function build_kcm()
{
	echo "==============================================="
	echo "  *** build KCM applicatin ***  "
	echo "==============================================="

	cd ${KCM_BUILD_DIR}
	
	make clean
	make all

	cd ${ROOT_DIR}
}

function build_shn()
{
	echo "==============================================="
	echo "  *** build SHN applicatin ***  "
	echo "==============================================="

	cd ${SHN_BUILD_DIR}
	
	make clean
	make all

	cd ${ROOT_DIR}
}

function make_pkg()
{
	echo "==============================================="
	echo "  *** make package ***  "
	echo "==============================================="

	cd ${ROOT_DIR}

	rm $PKG_NAME
	./pack

	cd ${ROOT_DIR}
}

	BUILD_OPTION="c"
	while [ $BUILD_OPTION != "q" ]
	do  
		cd $ROOT_DIR
		echo "==============================================="
		echo "  *** MTM Project Build ***  "
		echo ""
		echo "h) build HYUNDAE "
		echo ""
		echo "s) build SAMSUNG "
		echo ""
		echo "k) build KOCOM "
		echo ""
		echo "e) build EzVille "
		echo ""
		echo "q) quit "
		echo ""
		echo "==============================================="
		read -p "Build option : " BUILD_OPTION

		case "$BUILD_OPTION" in

		h)
		build_hdt
		cp  ${HDT_BIN_DIR}/$BIN_NAME ${ROOT_DIR}
		make_pkg
		;;

		s)
		build_shn
		cp  ${SHN_BIN_DIR}/$BIN_NAME ${ROOT_DIR}
		make_pkg
		;;

		k)
		build_kcm
		cp  ${KCM_BIN_DIR}/$BIN_NAME ${ROOT_DIR}
		make_pkg
		;;

		e)
		build_ezv
		cp  ${EZV_BIN_DIR}/$BIN_NAME ${ROOT_DIR}
		make_pkg
		;;

		q)
		echo " ---------------- end ----------------"
		;;

		esac
	done

	cd $ROOT_DIR
