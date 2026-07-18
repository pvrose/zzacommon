/*
	Copyright 2026, Philip Rose, GM3ZZA

	Test application for zc_graph_bar_horizontal widget.
*/

#include "zc_graph_.h"
#include "zc_line_style.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Enumerations.H>

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

// Function to convert integer to Roman numeral
std::string to_roman(int num) {
	const struct { int value; const char* numeral; } roman_data[] = {
		{10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}
	};

	std::string result;
	for (const auto& r : roman_data) {
		while (num >= r.value) {
			result += r.numeral;
			num -= r.value;
		}
	}
	return result;
}

// Generate random value between min and max
double random_value(double min, double max) {
	return min + (max - min) * (static_cast<double>(rand()) / RAND_MAX);
}

int main(int argc, char** argv) {
	// Initialize random seed
	srand(static_cast<unsigned int>(time(nullptr)));

	// Create main window
	Fl_Window* window = new Fl_Window(800, 600, "Horizontal Bar Chart Test");

	// Create the bar graph widget
	zc_graph_bar_horizontal* graph = new zc_graph_bar_horizontal(10, 10, 780, 580, "Test Bar Chart");
	graph->color(FL_WHITE);
	graph->textcolor(FL_BLACK);
	graph->textfont(FL_HELVETICA);
	graph->textsize(12);

	// Start configuration
	graph->start_config();

	// Create Roman numeral labels (I to XX = 1 to 20)
	std::vector<std::string> labels;
	for (int i = 1; i <= 20; i++) {
		labels.push_back(to_roman(i));
	}

	// Set up axis 0 (vertical axis) with bar labels
	graph->set_bar_labels(0, labels, 0.2, 0.5);
	graph->set_axis_params(0, zc_graph_::NO_MODIFIER, "", "Items", 40);

	// Set up axis 1 (horizontal axis) for data values
	graph->set_axis_params(1, zc_graph_::NO_MODIFIER, "", "Value", 50);
	graph->set_axis_ranges(1,
		zc_range<double>(),           // No inner range (auto-scale)
		zc_range<double>(0.0, 60.0),  // Outer range: 0 to 60
		zc_range<double>(0.0, 50.0)   // Default range: 0 to 50
	);

	// Create first data set with random values
	std::vector<zc_graph_::data_point_t>* data_set_1 = new std::vector<zc_graph_::data_point_t>();
	for (int i = 0; i < 20; i++) {
		double value = random_value(1.0, 50.0);
		data_set_1->push_back({ static_cast<double>(i), value });
	}

	// Create second data set with random values
	std::vector<zc_graph_::data_point_t>* data_set_2 = new std::vector<zc_graph_::data_point_t>();
	for (int i = 0; i < 20; i++) {
		double value = random_value(1.0, 50.0);
		data_set_2->push_back({ static_cast<double>(i), value });
	}

	// Create line styles for the data sets
	zc_line_style style1;
	style1.colour = FL_BLUE;
	style1.width = 2;
	style1.style = FL_SOLID;

	zc_line_style style2;
	style2.colour = FL_RED;
	style2.width = 2;
	style2.style = FL_SOLID;

	// Add data sets to the graph
	graph->add_data_set(1, data_set_1, style1);
	graph->add_data_set(1, data_set_2, style2);

	// End configuration
	graph->end_config();

	window->end();
	window->show(argc, argv);

	int result = Fl::run();

	// Clean up
	delete data_set_1;
	delete data_set_2;

	return result;
}