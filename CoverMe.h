#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <unordered_map>
#include <string.h>

using namespace std;

// Enum class for the Vaad options (as of 2019)
enum class Vaad_enum : char { TF, PE, YM, OTHER, NONE };

// Forward Declarations
class Vaad;
class Perek;
class Schedule;


class Person {
public:
	Person() {};

	Person(string name_in, bool maavar_in, vector<string> off_prefs_in, vector<string> vaad_prefs_in, Vaad * vaad_in) {
		name = name_in;
		maavar = maavar_in;
		off_prefs = off_prefs_in;
		vaad_prefs = vaad_prefs_in;
		vaad = vaad_in;
	} // Constructor



private:
	bool vaad_assigned = false;
	bool off_assigned = false;
	bool complete = false;
	
	bool on_maav = false;

	bool maavar = false;

	string name;
	vector<string> off_prefs;
	vector<string> vaad_prefs;

	Vaad * vaad;

	friend class Perek;
	friend class Schedule;

}; // Person class

class Vaad {
public:
	Vaad() {};
	Vaad(Vaad_enum vaad_in) {
		vaad = vaad_in;
		num_people++;
	} // Constructor


	void add_one() {
		num_people++;
	}

private:
	bool assigned = false;
	
	Vaad_enum vaad;
	int num_people = 0;

	string meeting_perek;
	vector<Person> members;

	friend class Person;
	friend class Perek;
	friend class Schedule;

}; // Vaad Class

class Perek {

public:
	Perek() {};
	Perek(int num_people_in, int order_in, string name_in, string time_in) {

		num_needed = num_people_in;
		order = order_in;
		name = name_in;
		time = time_in;

	} // Constructor



private:
	bool is_full = false;
	bool has_vaad = false;

	Vaad_enum vaad_assigned = Vaad_enum::NONE;
	
	int num_needed;
	int max_off;
	int num_off = 0;
	int order;

	string name;
	string time;

	vector<Person> on;
	vector<Person> vaad;
	vector<Person> off;

	friend class Schedule;

}; // Perek class

class Schedule {
public:
	Schedule() {}; // Only needs default constructor
	
	void read_input() {
	
		// Open file and check success
		ifstream input_file;
		input_file.open("CoverMe.in");
		if (!input_file.is_open()) {
			cout << "File invalid" << endl;
			exit(1);
		}

		// Read in the day number
		string junk;
		int day_num;

		// Day number: 3
		input_file >> junk >> junk >> day_num;

		date = day_num;

		int num_pereks_in;

		// Read in Perek data
		// Number of Pereks: 6
		input_file >> junk >> junk >> junk >> num_pereks_in;

		num_pereks = num_pereks_in;

		pereks.reserve(num_pereks);

		int num_people_in;
		string perek_name_in;
		string time_in;

		for (int i = 0; i < num_pereks; i++) {
		// Perek(int num_people_in, int order_in, string name_in, string time_in)

			input_file >> perek_name_in >> time_in >> num_people_in;
			pereks.emplace_back(num_people_in, i, perek_name_in, time_in);
			perek_map[perek_name_in] = &pereks[i];

		} // end loop

		int num_mav_in;

		// Read in maavar number
		// Number needed for Maavar: 2
		input_file >> junk >> junk >> junk >> junk >> num_mav_in;

		num_maavar = num_mav_in;

		int num_staff_in;

		input_file >> junk >> junk >> junk >> num_staff_in;

		num_people = num_staff_in;

		string name_in;
		string vaad_in;
		string bool_mav;


		// Read in Person data
		// Joey TF NO
		// OFF: Chug_B Limmud Chug_A Avodah Peulat Agam
		// VAAD : Chug_B Avodah Chug_A Limmud Peulat Agam
		for (int i = 0; i < num_people; i++) {
			input_file >> name_in >> vaad_in >> bool_mav;
			input_file >> junk;

			Vaad_enum vaad_en_in = vaad_helper(vaad_in);

			Vaad obj;
			Vaad * pointer;

			// Check the hash table for the vaad data, we want the pointers to be global for the people
			// Check back to make sure this works
			if (vaad_set.find(vaad_en_in) == vaad_set.end()) {
				obj = Vaad(vaad_en_in);
				pointer = &obj;
				vaad_set[vaad_en_in] = pointer;
			}
			else {
				pointer = vaad_set[vaad_en_in];
				pointer->add_one();
			}

			// Read in preferences
			vector<string> offs_in;
			offs_in.reserve(num_pereks);
			vector<string> vaads_in;
			vaads_in.reserve(num_pereks);


			for (int j = 0; j < num_pereks; j++) {
				input_file >> junk;
				offs_in.push_back(junk);
			
			} // end loop

			input_file >> junk;

			for (int j = 0; j < num_pereks; j++) {
				input_file >> junk;
				vaads_in.push_back(junk);

			} // end loop

			// Initialize the person object and add to the vector
			// Person(string name_in, bool maavar_in, vector<string> off_prefs_in, vector<string> vaad_prefs_in, Vaad * vaad_in)
			Person counselor = Person(name_in, maav_helper(bool_mav), 
				offs_in, vaads_in, pointer);
			people.push_back(counselor);
		} // end loop

	
		input_file.close(); // close file
	} // read_input

	void generate(){
	/*
	Steps:
	1. Assign maavar -> give maavar people their desired vaad and off
	1a. If no one wants maavar, assign from back to front
	2. Assign remaining Vaads
	3. Assign remaining Offs
	4. Output
	
	*/

	// get order
		start_order = get_order();

	// Assign maavar
		maavar_staff.reserve(num_maavar);
		num_maav_needed = num_maavar;
	
		// Traverse 
		int counter = start_order;
		for (int i = 0; i < num_people; i++) {

			if (num_maav_needed == 0) {
				break;
			}

			if (people[counter].maavar) {
				people[counter].on_maav = true;
				maavar_staff.push_back(&people[counter]);
				num_maav_needed--;
			}


			counter++;
			if (counter == num_people) {
				counter = 0;
			}
		}

		// assign people who didn't request maavar 
		if(num_maav_needed > 0) {
				assign_maavar();
		}

		// Give maav people their vaads and offs
		give_maav_prefs();
		

		// Give remaining vaads and offs
		give_remaining_vaads();
		give_remaining_offs();

	}; // generate

private:
	int start_order;
	int num_vaads = 4;
	int num_people;
	int num_maavar;
	int num_maav_req = 0;
	int num_maav_needed;
	int num_pereks;
	int date;
	vector<Person> people;
	//vector<Vaad> vaads;
	vector<Perek> pereks;
	vector<Person *> maavar_staff;

	unordered_map<Vaad_enum, Vaad *> vaad_set;
	unordered_map<string, Perek *> perek_map;


	int get_order() {
		// Returns the starting vaad for preference
		return date % num_people;
	}

	Vaad_enum vaad_helper(string string_in) {
		// String to enum convertor
		// Vaad_enum : char { TF, PE, YM, OTHER, NONE };
		if (string_in.compare("TF") == 0) {
			return Vaad_enum::TF;
		} else if (string_in.compare("PE") == 0) {
			return Vaad_enum::PE;
		} else if (string_in.compare("YM") == 0) {
			return Vaad_enum::YM;
		} else if (string_in.compare("OTHER") == 0) {
			return Vaad_enum::OTHER;
		} else {
			return Vaad_enum::NONE;
		}

	} // vaad helper

	bool maav_helper(string maav_in) {
		// string to bool conversion for maavar
		if (maav_in.compare("YES") == 0) {
			num_maav_req++; 
			return true;
		}
		return false;

	} // maavar helper

	void assign_maavar() {
	
		int counter = start_order - 1;
		for (int i = 0; i < num_maav_needed; i++) {

			if (counter == -1) {
				counter = num_people - 1;
			}

			if (num_maav_needed == 0) {
				break;
			}

			people[counter].on_maav = true;
			maavar_staff.push_back(&people[counter]);
			num_maav_needed--;
			


			counter--;
		}
	
	};

	void give_maav_prefs() {

		// give the vaads
		for (auto &counselor : maavar_staff) {

			give_vaad(*counselor);
			give_maav_offs(*counselor);
		}

	}

	void give_vaad(Person &counselor) {
		if (!counselor.vaad_assigned) {

			for (auto pref : counselor.vaad_prefs) {
				if (counselor.on_maav) {
					// Case where the counselor is on maavar
					// We check that the perek is free
					// and that it isn't a maavar perek
					if (!perek_map[pref]->has_vaad &&
						(perek_map[pref]->order > 2)) {
						counselor.vaad->meeting_perek = pref;
						counselor.vaad->assigned = true;
						counselor.vaad_assigned = true;
						perek_map[pref]->max_off = num_people - perek_map[pref]->num_needed - counselor.vaad->num_people; // calculate the max that can be off
						break;
					}
				} else {
					// Case for non-maavar
					if (!perek_map[pref]->has_vaad) {
						counselor.vaad->meeting_perek = pref;
						counselor.vaad->assigned = true;
						counselor.vaad_assigned = true;
						perek_map[pref]->max_off = num_people - perek_map[pref]->num_needed - counselor.vaad->num_people; // calculate the max that can be off
						break;
					}
				}
			}
		} else {
			counselor.vaad_assigned = true;
		}
	}

	void give_maav_offs(Person &counselor) {

		for (auto pref : counselor.off_prefs) {
			
			if (!counselor.off_assigned &&
				(perek_map[pref]->order > 2) && 
				(pref != counselor.vaad->meeting_perek)) {

				perek_map[pref]->off.push_back(counselor);
				perek_map[pref]->num_off++;
				counselor.off_assigned = true;
			}
			else {
				if (pref != counselor.vaad->meeting_perek) {
					perek_map[pref]->on.push_back(counselor);
				}
			}

		}

	}

	void give_off(Person &counselor) {

		for (auto pref : counselor.off_prefs) {

			if (!counselor.off_assigned &&
				((int)perek_map[pref]->on.size() < perek_map[pref]->max_off) &&
				(pref != counselor.vaad->meeting_perek)) {

				perek_map[pref]->off.push_back(counselor);
				perek_map[pref]->num_off++;
				counselor.off_assigned = true;
			}
			else {
				if (pref != counselor.vaad->meeting_perek) {
					perek_map[pref]->on.push_back(counselor);
				}
			}

		}
	}

	void give_remaining_vaads() {

		int counter = start_order;
		for (int i = 0; i < num_people; i++) {

			give_vaad(people[counter]);

			counter++;
			if (counter == num_people) {
				counter = 0;
			}
		}

	}

	void give_remaining_offs() {
	
		int counter = start_order;
		for (int i = 0; i < num_people; i++) {

			give_off(people[counter]);

			counter++;
			if (counter == num_people) {
				counter = 0;
			}
		}	
	};

};