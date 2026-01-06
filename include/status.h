#ifndef __STATUS__
#define __STATUS__

#include "drawing.h"
#include "utils.h"

#include <map>
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <cstdint>


//! Forwrad declaration.
class banner;

	//! The colours used for a particular status_t value.
	struct colours_t {
		Fl_Color fg;          //!< Foreground colour
		Fl_Color bg;          //!< Background colour
	};

	//! The status of the various messages.
	enum status_t : char {
		ST_NONE,             //!< Uninitialised
		ST_LOG,              //!< Only log the message, do not display it in status
		ST_DEBUG,            //!< Debug message
		ST_NOTE,             //!< An information message
		ST_PROGRESS,         //!< A progress note
		ST_OK,               //!< Task successful
		ST_WARNING,          //!< A warning message
		ST_ERROR,            //!, An error has been signaled
		ST_SEVERE,           //!< A sever error that will result in reduced capability
		ST_FATAL             //!< A fatal (non-recoverable) error has been signaled
	};

	//! Map the values of status_t to the colours used to display them.
	const std::map<status_t, colours_t> STATUS_COLOURS = {
		{ ST_NONE, { FL_WHITE, FL_BLACK } },
		{ ST_LOG, { fl_lighter(FL_BLUE), FL_BLACK } },
		{ ST_DEBUG, { fl_lighter(FL_MAGENTA), FL_BLACK } },
		{ ST_NOTE, { fl_lighter(FL_CYAN), FL_BLACK } },
		{ ST_PROGRESS, { fl_darker(FL_WHITE), FL_BLACK } },
		{ ST_OK, { fl_lighter(FL_GREEN), FL_BLACK } },
		{ ST_WARNING, { FL_YELLOW, FL_BLACK } },
		{ ST_ERROR, { FL_RED, FL_BLACK } },
		{ ST_SEVERE, { FL_RED, FL_WHITE } },
		{ ST_FATAL, { FL_BLACK, FL_RED } },
	};

	//! Code - letters witten to log file to indicate severity of the logged status
	const std::map<status_t, char> STATUS_CODES = {
		{ ST_NONE, ' '},
		{ ST_LOG, 'L'},
		{ ST_DEBUG, 'B' },
		{ ST_NOTE, 'N'},
		{ ST_PROGRESS, 'P'},
		{ ST_OK, 'D'},
		{ ST_WARNING, 'W'},
		{ ST_ERROR, 'E'},
		{ ST_SEVERE, 'S'},
		{ ST_FATAL, 'F'}
	};

	//! Status object parameters
	struct object_data_t {
		const char* name;              //!< Text to use in output
		Fl_Color colour;               //!< Colour for text and progress wheel
	};

	//! Object parameter map
	typedef std::map<uint8_t, object_data_t> object_data_map;

	//! This class provides the means of managing status and progress for the user application.
	class status 
	{
	public:

		//! \brief Constructor.
		//! \param features The features supported. The options currently are given below.
		//! \param data_map Formatting parameters.
	    //! \par HAS_BANNER: 
		//! Open a window of class banner.
		//! \par HAS_LOGFILE: 
		//! Write status messages to a file.
		//! \par HAS_CONSOLE: 
		//! Write status messages to the console.
		status(uint8_t features, const object_data_map& data_map);
		static const uint8_t HAS_BANNER = 1;          //!< Opens a banner window
		static const uint8_t HAS_LOGFILE = 2;         //!< Writes to a log file
		static const uint8_t HAS_CONSOLE = 4;         //!< Displays on the console

     	//! Destructor.
		~status();

		//! \brief Initialise progress.
		//! \param max_value Maximum value of items being used to monitor progress.
		//! \param object An identifier for the what is being measured
		//! \param description Textual description of what is being measured.
		//! \param suffix Indicates units of items being measured.
		void progress(uint64_t max_value, uint8_t object, const char* description, const char* suffix);
		
		//! \brief Update progress.		
		//! \param value Current number of items being measured.
		//! \param object Identifier.
		void progress(uint64_t value, uint8_t object);

		//! \brief Update progress with a text message and mark complete.		
		//! \param message Message indicating why it is complete.
		//! \param object Identifier.
		void progress(const char* message, uint8_t object);

		//! Output message \p label with \p status.
		void misc_status(status_t status, const char* label);

		//! \brief Set close-down callback.
		//! \param w Pointer to the widget that will action the close-down.
		//! \param close Pointer to the function that will action the close-down.
		void callback(Fl_Window* w, void(*close)(Fl_Window* w, void* v));


		//! \brief Initiate a close-down (from banner).
		void close();

		//! \brief Returns pointer to the banner
		banner* get_banner();

	protected:
		//! Returns the terminal characters used for the specific \p fg or bg colour for \p status.
		std::string colour_code(status_t status, bool fg); 


	protected:
		//! Status report file.
		std::string report_filename_ = nullptr;
		//! Output stream for report file.
		std::ofstream* report_file_ = nullptr; 
		//! Report file unusable.
		bool file_unusable_ = false;

		//! Callback to initiate close-down.
		friend class banner;
		void(*close_)(Fl_Window* w, void* v);
		//! Closing window
		Fl_Window* window_ = nullptr;

		//! Features
		uint8_t feature_set_ = 0;

		//! Banner
		banner* banner_;

		//! Keep banner when closing
		bool keep_banner_ = false;

		//! Display parameter map
		object_data_map object_map_;
	};

	extern status* status_;
#endif

