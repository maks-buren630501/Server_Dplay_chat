#pragma once

#include"dplay8.h"
#include<iostream>

using namespace std;

struct User
{
	DPNID id;
	string name;
};

struct Message
{
	DPNID id;
	char name[64];
	char data[256];
};