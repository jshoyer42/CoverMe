#include <iostream>
#include <string>
#include <string.h>
#include <iomanip>
#include "CoverMe.h"

using namespace std;


int main() {
	ios_base::sync_with_stdio(false);

	Schedule sched;
	sched.read_input();
	sched.generate();

	return 0;
}