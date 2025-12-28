/*! \file 
Standard drawing types and constants for use in views and dialogs.

*/

#ifndef __DRAWING__
#define __DRAWING__

#include <map>

#include<FL/Enumerations.H>


// Default font-size to use (& override FLTK defaults?)
const int DEFAULT_SIZE = 10;                   //!< Default font size
const Fl_Fontsize FONT_SIZE = DEFAULT_SIZE;    //!< Default font size
const Fl_Font FONT = FL_HELVETICA;             //!< Default font

// Main window default sizes
const unsigned int MENU_HEIGHT = 30;           //!< Height of menu bar
const unsigned int TOOL_HEIGHT = 20;           //!< Height of tool bar
const unsigned int FOOT_HEIGHT = 15;           //!< Height of fotter - used for copyright statements in windows.
const unsigned int TOOL_GAP = 5;               //!< Gap between groups of toolbar items
const unsigned int BORDER_SIZE = 5;            //!< Width of window borders
const unsigned int TAB_HEIGHT = 20;            //!< 

// drawing constants
const int EDGE = 10;                        //!< Gap between group edge and widget edge
const int HBUTTON = 20;                     //!< Height of a normal button
const int WBUTTON = 60;                     //!< Width of a normal button
const int XLEFT = EDGE;                     //!< Start of widgets
const int YTOP = EDGE;                      //!< Start of widgets
const int GAP = 10;                         //!< Gap between non-related widgets
const int HTEXT = 20;                       //!< Gap to leave for text
const int WRADIO = 15;                      //!< Width of a box-less rado button
const int HRADIO = WRADIO;                  //!< Height of a boxless button
const int WLABEL = 50;                      //!< gap for a label outwith widget
const int WLLABEL = 100;                    //!< gap for a large label outwith widget
const int HMLIN = 3 * HTEXT;                //!< Height of a multi-line text box
const int WEDIT = 3 * WBUTTON;              //!< Width of a text edit box
const int WSMEDIT = 2 * WBUTTON;            //!< Width of a small text edit box
const int ROW_HEIGHT = DEFAULT_SIZE + 4;    //!< Default height for table rows

// Colours to use for buttons - defined using FLTK colour palette
const Fl_Color COLOUR_ORANGE = 93;       /*!< R=4/4, B=0/4, G=5/7 */
const Fl_Color COLOUR_APPLE = 87;        /*!< R=3/4, B=0/4, G=7/7 */
const Fl_Color COLOUR_PINK = 170;        /*!< R=4/4, B=2/4, G=2/7 */
const Fl_Color COLOUR_MAUVE = 212;       /*!< R-4/4, B=3/4, G=4/7 */
const Fl_Color COLOUR_NAVY = 136;        /*!< R=0/4, B=2/4, G=0/7 */
const Fl_Color COLOUR_CLARET = 80;       /*!< R=3/4, B=0/4, G=0/4 */
const Fl_Color COLOUR_GREY = fl_color_average(FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR, 0.33F);
	//!< One third between fotreground and background colours.


#endif
