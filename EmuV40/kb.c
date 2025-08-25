#include "kb.h"
#include <stdio.h>

bool keys[256];

void handle_keyboard(bool down, SDL_Keycode sym, SDL_Scancode scancode) {

	/*if (scancode == SDL_SCANCODE_LSHIFT || scancode == SDL_SCANCODE_RSHIFT) {

	}
	else {
		keys[sym] = down;
	}*/
	
	if (sym >= 32 && sym <= 127) {
		keys[sym] = down;
	}

	
}

//char getUnicodeValue(const SDL_KeyboardEvent *key) {    
//	// magic numbers courtesy of SDL docs :)    
//	const int INTERNATIONAL_MASK = 0xFF80, UNICODE_MASK = 0x7F;    
//	int uni = key->keysym.unicode;    
//	if( uni == 0 ) // not translatable key (like up or down arrows)    
//	{        // probably not useful as string input        
//			 // we could optionally use this to get some value        
//			 // // for it: 
//			 SDL_GetKeyName( key );        
//			 return 0;   
//	}    
//	
//	if( ( uni & INTERNATIONAL_MASK ) == 0 )    
//	{        
//		if( SDL_GetModState() & KMOD_SHIFT )        
//		{            
//			return (char)(toupper(uni & UNICODE_MASK));        
//		}        
//		else        
//		{            
//			return (char)(uni & UNICODE_MASK);        
//		}    
//	}    
//	else // we have a funky international character. one we can't read :(    
//	{        // we could do nothing, or we can just show some sign of input, like so:        
//		return '?';    
//	}
//}