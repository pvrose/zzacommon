#include "zc_ticker.h"

//#include "main.h"

#include <algorithm> 

#include <FL/Fl.H>

const double TICK = 0.1;
extern bool DEBUG_QUICK;

// Constructor
zc_ticker::zc_ticker() {
    tick_count_ = 0;
    Fl::add_timeout(TICK, cb_ticker, this);
}

// Destructor
void zc_ticker::stop_all() {
    Fl::remove_timeout(cb_ticker);
    for( auto it = tickers_.begin(); it != tickers_.end(); it++) {
        delete *it;
    };
    tickers_.clear();
}

zc_ticker::~zc_ticker() {
    stop_all();
}

// Add zc_ticker
void zc_ticker::add_ticker(void* object, callback* cb, unsigned int interval, bool immediate) {
    ticker_entry* entry = new ticker_entry;
    entry->object = object;
    entry->tick = cb;
    entry->period_ds = DEBUG_QUICK ? std::min<unsigned int>(interval, 3000U) : interval;
    entry->active = true;
    entry->not_ticked = immediate;
    tickers_.push_back(entry);
}

// Remove ticker
void zc_ticker::remove_ticker(void* object) {
    bool go_on = true;
    auto itd = tickers_.begin();
    for (auto it = tickers_.begin(); it != tickers_.end() && go_on; it++) {
        if ((*it)->object == object) {
           go_on = false;
           itd = it;
        }
    }
    if (!go_on) {
        // We are removing this. it should be safe as we clear go_on
        tickers_.erase(itd);
    }
}

// Activate ticker
void zc_ticker::activate_ticker(void* object, bool active) {
    bool go_on = true;
    auto it = tickers_.begin();
    for (; it != tickers_.end() && go_on; it++) {
        if ((*it)->object == object) {
            go_on = false;
            (*it)->active = active;
        }
    }
}

// Call back - check tickers
void zc_ticker::cb_ticker(void * v) {
    zc_ticker* that = (zc_ticker*)v;
    that->tick_count_++;
    for (auto it = that->tickers_.begin(); it != that->tickers_.end(); it++) {
        // Send ticks to all who need it at this time
        if ((that->tick_count_ % (*it)->period_ds == 0) || (*it)->not_ticked) {
            if ((*it)->active) {
                (*it)->tick((*it)->object);
                (*it)->not_ticked = false;
            }
        }
    }
    // Now repeat timer
    Fl::repeat_timeout(TICK, cb_ticker, v);
}