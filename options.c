#include "options.h"

struct option longopts[] = {
    { "input_font", required_argument, NULL, 'i' },
    { "output_font", required_argument, NULL, 'o' },
    { "output_font_type", required_argument, NULL, 't' },	// c, b
    { "width", required_argument, NULL, 'W' },
    { "height", required_argument, NULL, 'H' },
    { "code_custom_start", required_argument, NULL, 's' },
    { "code_custom_end", required_argument, NULL, 'e' },
    { "code_kor", no_argument, NULL, 'K' },
    { "code_jpn", no_argument, NULL, 'J' },
    { "code_chn", no_argument, NULL, 'C' },
    { "code_rus", no_argument, NULL, 'R' },
    { "code_eng", no_argument, NULL, 'E' },	
    { "no_align", no_argument, NULL, 'n' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 }};



struct opt g_opt;
