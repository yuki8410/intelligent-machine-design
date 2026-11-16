#!/bin/bash

make clean
if [ $? -ne 0 ]; then
  echo "make clean に失敗しました。"
  exit 1
fi

make
if [ $? -ne 0 ]; then
  echo "make に失敗しました。"
  exit 1
fi

make run
if [ $? -ne 0 ]; then
	echo "make run に失敗しました。"
	exit 1
fi

make rmse
if [ $? -ne 0 ]; then
  echo "make rmse に失敗しました。"
  exit 1
fi

make show3d
if [ $? -ne 0 ]; then
  echo "make show3d に失敗しました。"
  exit 1
fi

make correct
