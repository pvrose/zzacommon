/*
	Copyright 2017-2026, Philip Rose, GM3ZZA
	
    This file is part of ZZACOMMON.

    ZZACOMMON is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

    ZZACOMMON is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
	PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZACOMMON. 
	If not, see <https://www.gnu.org/licenses/>. 

*/
#pragma once

#include <cstdint>
#include <mutex>
#include <vector>




//! \brief This class provides a central timer to control all the real time activity.
//! 
//! Any object that wants a regular tick
//! subscribes to the ticker by providing a callback and a period.
//! The minimum period is 100 milliseconds, and the period is specified in units of 100 milliseconds.
class zc_ticker {

    //! Ticker callback.
    typedef void callback(void* v);

    //! Entry registering an object's request for a tick.
    struct ticker_entry {
        void* object { nullptr };     //!< Pointer to the object - gets returned to the object
        callback* tick { nullptr };   //!< The static function to call
        unsigned int period_ds{ 0 };  //!< Period in units of 100 milliseconds
        bool active{ false };         //!< Ticker active
        bool not_ticked{ true };      //!< Ticker not yet sent - so send it regardless
    };

    public:
    
    //! Constructor
    zc_ticker();
    //! Destructor
    ~zc_ticker();

    //! \brief Set ticker.    
    //! \param object Pointer to the object requesting a tick.
    //! \param cb Method to use as callback.
    //! \param interval Tick period in units of 100 milliseconds.
    //! \param immediate If true, immediately issues a callback to the requesting unit and 
    //! subsequenly every \p interval.
    void add_ticker(void* object, callback* cb, unsigned int interval, bool immediate = true);

    //! Remove ticker for \p object.
    void remove_ticker(void* object);

    //! \brief Suspend/restart ticker.    
    //! \param object Pointer to the requesting object.
    //! \param active If true restart the ticker otherwise suspend it.
    void activate_ticker(void* object, bool active);

    //! Stop all tickers.
    void stop_all();
    
    protected:

    //! Callback that gets repeated every 100 milliseconds.
    static void cb_ticker(void* v);

    //! The register of tick requests.
    std::vector<ticker_entry*> tickers_;
    
    //! Current time (in units of 100 milliseconds since started).
    unsigned int tick_count_;

	//! Lock to coordinate stopping of ticker and processing of ticks.
	std::mutex stop_lock_;
    
};

extern zc_ticker* ticker_;