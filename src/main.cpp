#include "user.h"
#include "archive.h"
#include "archive_io.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <regex>
#include <string>

namespace fs = boost::filesystem;
namespace po = boost::program_options;
using std::string;

void parse_date_and_index(string const& arg, string& date, int& index)
{
	std::regex reg("(.+):(.+)");
	std::smatch match;
	if(!std::regex_match(arg, match, reg)) {
		throw po::error("invalid date and index: " + arg);
	}

	date = match[1];
	index = std::stoi(match[2]);
}

int main(int argc, char** argv)
{
	string write;
	string date_to_write;
	string read{"last"};
	string edit{"last:-1"};
	string remove{"last:-1"};

	po::options_description desc("notes application usage");
	desc.add_options()
		("help,h", "print this message")
		("write,w", po::value<string>(&write), "note to write to archive")
		("date,d", po::value<string>(&date_to_write), "combined with --write; at this date note will be written")
		("read,r", po::value<string>(&read), "date that will be readed")
		("edit", po::value<string>(&edit), "date and index in archive that will be edited")
		("remove", po::value<string>(&remove), "date and index in archive that will be removed")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.empty() || vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	if(vm.count("edit")) {
		if(!vm.count("write")) {
			throw po::required_option("write");
		}

		string date;
		int index;

		parse_date_and_index(edit, date, index);

		fs::path path = notes::parse_date_as_path(date);

		notes::Archive ar;
		notes::load(ar, path);

		auto it = ar.index(index);

		notes::Note temp(*it);

		ar.remove(it);

		temp = write;

		ar.add(std::move(temp));

		notes::save(ar, path);

	} else if(vm.count("remove")) {
		notes::Archive ar;

		string date;
		int index;
		parse_date_and_index(remove, date, index);

		auto path = notes::parse_date_as_path(date);

		notes::load(ar, path);

		auto it = ar.index(index);

		ar.remove(it);

		notes::save(ar, path);

	} else if(vm.count("read")) {
		notes::Archive ar;
		auto path = notes::parse_date_as_path(read);

		notes::load(ar, path);

		std::cout << ar << std::endl;

	} else if(vm.count("write")) {
		notes::Archive ar;
		fs::path path;
		time_t t = time(0);

		if(vm.count("date")) {
			path = notes::parse_date_as_path(date_to_write);
		} else {
			path = notes::make_path_from_date(t);
		}

		if(fs::exists(path)) {
			notes::load(ar, path);
		}

		ar.add(write, t);

		notes::save(ar, path);
	}
}
