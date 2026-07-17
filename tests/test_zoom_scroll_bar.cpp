/*
	Test application for zc_zoom_scroll_bar widget

	This demonstrates a horizontal zoom scroll bar with value output.

	Copyright 2026, Philip Rose, GM3ZZA
*/

#include "zc_zoom_scroll_bar.h"
#include "zc_range.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Enumerations.H>

#include <string>
#include <sstream>
#include <iomanip>

// Global output box to display current values
Fl_Box* output_box = nullptr;

// Callback function for the zoom scroll bar
void zoom_scroll_callback(Fl_Widget* w, void* data) {
	auto* bar = static_cast<zc_zoom_scroll_bar*>(w);

	if (output_box) {
		zc_range<double> current_value = bar->value();
		zc_range<double> bounds = bar->bounds();
		double slider_size = bar->slider_size();

		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2);
		oss << "Current Range:\n";
		oss << "  Min: " << current_value.first << "\n";
		oss << "  Max: " << current_value.second << "\n";
		oss << "  Span: " << current_value.size() << "\n\n";
		oss << "Total Bounds:\n";
		oss << "  Min: " << bounds.first << "\n";
		oss << "  Max: " << bounds.second << "\n";
		oss << "  Span: " << bounds.size() << "\n\n";
		oss << "Slider Size (fraction of total): " << slider_size << "\n";
		oss << "Zoom Level: " << std::setprecision(1)
			<< (bounds.size() / current_value.size()) << "x\n\n";
		oss << "Instructions:\n";
		oss << "- Drag slider to scroll\n";
		oss << "- Mouse wheel to zoom\n";
		oss << "- Zoom centers on cursor";

		output_box->copy_label(oss.str().c_str());
	}
}

int main(int argc, char** argv) {
	// Create main window
	Fl_Window* window = new Fl_Window(600, 300, "zc_zoom_scroll_bar Test");

	// Create the zoom scroll bar (horizontal)
	zc_zoom_scroll_bar* scroll_bar = new zc_zoom_scroll_bar(20, 30, 560, 30, "Horizontal Zoom Scroll Bar");
	scroll_bar->align(FL_ALIGN_TOP_LEFT);
	scroll_bar->type(FL_HORIZONTAL);
	scroll_bar->color(FL_BACKGROUND_COLOR);
	scroll_bar->selection_color(FL_SELECTION_COLOR);

	// Set up the range: total bounds from 0 to 1000
	scroll_bar->bounds(zc_range<double>(0.0, 1000.0));

	// Set initial visible range from 200 to 400
	scroll_bar->value(zc_range<double>(200.0, 400.0));

	// Set callback
	scroll_bar->callback(zoom_scroll_callback);

	// Create output display box
	output_box = new Fl_Box(20, 80, 560, 200);
	output_box->box(FL_DOWN_BOX);
	output_box->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	output_box->labelfont(FL_COURIER);
	output_box->labelsize(12);

	// Initial update of output
	zoom_scroll_callback(scroll_bar, nullptr);

	window->end();
	window->show(argc, argv);

	return Fl::run();
}