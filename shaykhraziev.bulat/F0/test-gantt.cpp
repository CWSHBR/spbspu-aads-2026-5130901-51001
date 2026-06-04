#include <boost/test/unit_test.hpp>

#include "gantt.hpp"
#include "planner.hpp"
#include "project.hpp"

#include <sstream>

BOOST_AUTO_TEST_CASE(gantt_renders_typical_project)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("layout", 2, "Layout");
  project.addTask("backend", 4, "Backend");
  project.addTask("test", 2, "Test");
  project.addDependency("layout", "design");
  project.addDependency("backend", "design");
  project.addDependency("test", "layout");
  project.addDependency("test", "backend");
  shaykhraziev::buildProjectPlan(project);
  std::ostringstream out;

  shaykhraziev::renderGantt(project, out);

  BOOST_TEST(out.str() ==
      "<GANTT site>\n"
      "DAYS:        1 2 3 4 5 6 7 8 9\n"
      "design   W1 # # #            \n"
      "layout   W1       # #        \n"
      "backend  W2       # # # #    \n"
      "test     W1               # #\n"
      "<PROJECT-END: 9>\n");
}

BOOST_AUTO_TEST_CASE(gantt_aligns_different_task_id_lengths)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("a", 1, "A");
  project.addTask("longtask", 1, "Long");
  shaykhraziev::buildProjectPlan(project);
  std::ostringstream out;

  shaykhraziev::renderGantt(project, out);

  BOOST_TEST(out.str() ==
      "<GANTT site>\n"
      "DAYS:         1\n"
      "a         W1 #\n"
      "longtask  W2 #\n"
      "<PROJECT-END: 1>\n");
}

BOOST_AUTO_TEST_CASE(gantt_aligns_marks_under_two_digit_days)
{
  shaykhraziev::Project project("site", 23, 2);
  project.addTask("short", 1, "Short");
  project.addTask("long", 5, "Long");
  shaykhraziev::buildProjectPlan(project);
  std::ostringstream out;

  shaykhraziev::renderGantt(project, out);

  BOOST_TEST(out.str() ==
      "<GANTT site>\n"
      "DAYS:      23 24 25 26 27\n"
      "short  W1  #            \n"
      "long   W2  #  #  #  #  #\n"
      "<PROJECT-END: 27>\n");
}
