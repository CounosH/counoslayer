#!/usr/bin/env bash

counosh-cli stop
cd $HOME/counos/
make
counoshd -daemon -fallbackfee=0.00001
tail -f ~/.counosh/debug.log