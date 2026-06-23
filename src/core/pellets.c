#include "pellets.h"

int bind_pellets_files(pellet_t *pellets){
    pellets[0].filepath = "assets/files/1.txt"; 
    pellets[0].arc_type = TXT;
    pellets[1].filepath = "assets/files/2.txt"; 
    pellets[1].arc_type = TXT;
    
    pellets[2].filepath = "assets/files/3.jpg"; 
    pellets[2].arc_type = JPG;
    pellets[3].filepath = "assets/files/4.jpg"; 
    pellets[3].arc_type = JPG;
    
    pellets[4].filepath = "assets/files/5.mp4"; 
    pellets[4].arc_type = MP4;
    pellets[5].filepath = "assets/files/6.mp4"; 
    pellets[5].arc_type = MP4;

    return 0;
}