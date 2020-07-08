#pragma once
#include "pch.h"
#include "Random.h"
#include <time.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

//----------------------------------------------------------------------
//initialization of the static member variable must be done in the .cpp file.
//
int Random::objectCount = 0;
//
//----------------------------------------------------------------------


Random::Random() {
	objectCount++;
	if (objectCount == 1) {
		//cout<<endl<<"random number generator seeded..."<<endl;
		//-----------------------------------------------
		srand((unsigned int)time(0));
	}
}

int Random::GetNext(int lo, int hi) {
	int r = rand() % (hi - lo + 1) + lo;
	return r;

}
