#!/bin/bash


cp PGStarter/DM/* ../../../DataModel/ 

more ../../../DataModel/DataModel.h | sed s:'#include "Store.h"':'#include "Store.h"\n#include "PSQLInterface.h"': | sed s:'private\:':' PSQLInterface SQL;\n\nprivate\:': > ../../../DataModel/DataModel.h.new
mv ../../../DataModel/DataModel.h.new ../../../DataModel/DataModel.h
