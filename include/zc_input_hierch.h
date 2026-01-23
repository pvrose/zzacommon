#pragma once
#include <FL/Fl_Input_Choice.H>

//! \brief This class inherits from Fl_Input_Choice but presents the choice button menu
//! in hierarchic form to manage a large selection range.
class zc_input_hierch :
    public Fl_Input_Choice
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	zc_input_hierch(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
	~zc_input_hierch();

	//! Overload inherited value - strips hierarchic path after calling Fl_Input_Choice::value().
	const char* value();
	//! Overload inherited value - adds the hierarchic path before writing value().
	void value(const char* val);

	//! Overload Fl_Input_Choice::add(const char* s) - adds hierarchic path.
	void add(const char* val);

	//! \brief Set the hierarchy description ,
	//! \param start Position in value to start adding hierarchy
	//! \param end Position in value to end adding hierarchy
	//! For example if start is set to 0 and end to 2, the value "GM3ZZA" will be
	//! changed to "G/GM/GM3ZZA" to give a three-level hierarchic menu.
	void hierarchy(int start, int end);
	//! \brief Get the hierarchy description.	
	//! \see hierarchy
	void hierarchy(int* start, int* end) const;

protected:
	//! Returns the \p val prefixed with its hierarchic path
	const char* hierise(const char* val) const;

	//! Position in value string to start adding hierarchy.
	int start_;
	//! Position in value string to end adding hierarchy.
	int end_;


};

