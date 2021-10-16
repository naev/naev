/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "nxml.h"

int nxml_persistLua( nlua_env env, xmlTextWriterPtr writer );
int nxml_unpersistLua( nlua_env env, xmlNodePtr parent );
