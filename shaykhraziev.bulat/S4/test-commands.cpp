#include <boost/test/unit_test.hpp>

#include "commands.hpp"

#include <sstream>

namespace
{
  shaykhraziev::DatasetTable makeDatasets()
  {
    shaykhraziev::DatasetTable datasets;
    shaykhraziev::Dictionary first;
    first.push(2, "two");
    first.push(1, "one");
    shaykhraziev::Dictionary second;
    second.push(2, "two-second");
    second.push(3, "three");
    shaykhraziev::Dictionary empty;

    datasets.push("first", first);
    datasets.push("second", second);
    datasets.push("empty", empty);
    return datasets;
  }

  std::string run(shaykhraziev::DatasetTable& datasets, const std::string& command)
  {
    std::ostringstream out;
    shaykhraziev::executeCommandLine(datasets, command, out);
    return out.str();
  }
}

BOOST_AUTO_TEST_CASE(commands_print_existing_dictionary)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();

  BOOST_TEST(run(datasets, "print first") == "first 1 one 2 two\n");
}

BOOST_AUTO_TEST_CASE(commands_print_empty_dictionary)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();

  BOOST_TEST(run(datasets, "print empty") == "<EMPTY>\n");
}

BOOST_AUTO_TEST_CASE(commands_print_unknown_dictionary_is_invalid)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();

  BOOST_TEST(run(datasets, "print missing") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_create_complement_intersect_union)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();

  BOOST_TEST(run(datasets, "complement only first second") == "");
  BOOST_TEST(run(datasets, "intersect both first second") == "");
  BOOST_TEST(run(datasets, "union all first second") == "");
  BOOST_TEST(run(datasets, "print only") == "only 1 one\n");
  BOOST_TEST(run(datasets, "print both") == "both 2 two\n");
  BOOST_TEST(run(datasets, "print all") == "all 1 one 2 two 3 three\n");
}

BOOST_AUTO_TEST_CASE(commands_replace_existing_new_dataset)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();

  BOOST_TEST(run(datasets, "union first second first") == "");
  BOOST_TEST(run(datasets, "print first") == "first 1 one 2 two-second 3 three\n");
}

BOOST_AUTO_TEST_CASE(commands_reject_missing_operand_dataset)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();

  BOOST_TEST(run(datasets, "union new first missing") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "intersect new missing first") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_reject_unknown_command)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();

  BOOST_TEST(run(datasets, "unknown first") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_reject_wrong_argument_count)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();

  BOOST_TEST(run(datasets, "print") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "union new first") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_registry_contains_handlers)
{
  shaykhraziev::CommandRegistry registry = shaykhraziev::makeCommandRegistry();

  BOOST_CHECK(registry.has("print"));
  BOOST_CHECK(registry.has("complement"));
  BOOST_CHECK(registry.has("intersect"));
  BOOST_CHECK(registry.has("union"));
  BOOST_CHECK(!registry.has("missing"));
  BOOST_TEST(registry.get("print").argumentCount == 1);
  BOOST_TEST(registry.get("complement").argumentCount == 3);
  BOOST_TEST(registry.get("intersect").argumentCount == 3);
  BOOST_TEST(registry.get("union").argumentCount == 3);
}

BOOST_AUTO_TEST_CASE(commands_processes_lines_until_eof)
{
  shaykhraziev::DatasetTable datasets = makeDatasets();
  std::istringstream in("print first\nbad\nprint empty\n");
  std::ostringstream out;

  shaykhraziev::processCommands(datasets, in, out);

  BOOST_TEST(out.str() == "first 1 one 2 two\n<INVALID COMMAND>\n<EMPTY>\n");
}
