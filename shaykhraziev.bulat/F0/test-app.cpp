#include <boost/test/unit_test.hpp>

#include "app.hpp"

#include <fstream>
#include <cstdio>
#include <sstream>
#include <string>

BOOST_AUTO_TEST_CASE(app_rejects_missing_file)
{
  std::istringstream in("");
  std::ostringstream out;
  std::ostringstream err;

  int result = shaykhraziev::runF0("__missing_f0_file__", in, out, err);

  BOOST_TEST(result == 1);
  BOOST_TEST(out.str() == "");
  BOOST_TEST(err.str() == "cannot open file\n");
}

BOOST_AUTO_TEST_CASE(app_prints_invalid_for_unknown_command)
{
  const char* filename = "/tmp/shaykhraziev_f0_empty.txt";
  {
    std::ofstream file(filename);
  }
  std::istringstream in("unknown\n\n");
  std::ostringstream out;
  std::ostringstream err;

  int result = shaykhraziev::runF0(filename, in, out, err);

  BOOST_TEST(result == 0);
  BOOST_TEST(out.str().find(" > Unknown command: unknown") != std::string::npos);
  BOOST_TEST(err.str() == "");
}

BOOST_AUTO_TEST_CASE(app_allows_start_without_file_and_creates_project_file)
{
  std::remove("site");
  std::istringstream in(
      "make-project site 1 2\n"
      "add-task site design 3 Design\n");
  std::ostringstream out;
  std::ostringstream err;

  int result = shaykhraziev::runF0(nullptr, in, out, err);

  BOOST_TEST(result == 0);
  BOOST_TEST(out.str().find(" > Project created: site") != std::string::npos);
  BOOST_TEST(err.str() == "");

  std::ifstream file("site");
  BOOST_REQUIRE(file);
  std::string saved;
  std::getline(file, saved);
  BOOST_TEST(saved == "project site 1 2");
  std::getline(file, saved);
  BOOST_TEST(saved == "task site design 3 Design");
  file.close();
  std::remove("site");
}

BOOST_AUTO_TEST_CASE(app_reads_and_writes_passed_file)
{
  const char* filename = "/tmp/shaykhraziev_f0_save.txt";
  {
    std::ofstream file(filename);
    file << "project site 1 2\n";
  }
  std::istringstream in("add-task site design 3 Design\n");
  std::ostringstream out;
  std::ostringstream err;

  int result = shaykhraziev::runF0(filename, in, out, err);

  BOOST_TEST(result == 0);
  BOOST_TEST(out.str().find("Task added: design") != std::string::npos);
  std::ifstream file(filename);
  std::string contents;
  std::string line;
  while (std::getline(file, line))
  {
    contents += line;
    contents += '\n';
  }
  BOOST_TEST(contents.find("project site 1 2\n") != std::string::npos);
  BOOST_TEST(contents.find("task site design 3 Design\n") != std::string::npos);
  std::remove(filename);
}
