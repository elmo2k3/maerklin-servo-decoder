#ifndef __MAIN_H__
#define __MAIN_H__

// this code is from peter dannegger (mikrocontroller.net)
#define debounce( port, pin )						\
({									\
  static uint8_t flag = 0;	/* new variable on every macro usage */	\
  uint8_t i = 0;							\
									\
  if( flag ){			/* check for key release: */		\
    for(;;){			/* loop ... */				\
      if( !(port & 1<<pin) ){	/* ... until key pressed or ... */	\
	i = 0;			/* 0 = bounce */			\
	break;								\
      }									\
      _delay_us( 98 );		/* * 256 = 25ms */			\
      if( --i == 0 ){		/* ... until key >25ms released */	\
	flag = 0;		/* clear press flag */			\
	i = 0;			/* 0 = key release debounced */		\
	break;								\
      }									\
    }									\
  }else{			/* else check for key press: */		\
    for(;;){			/* loop ... */				\
      if( (port & 1<<pin) ){	/* ... until key released or ... */	\
	i = 0;			/* 0 = bounce */			\
	break;								\
      }									\
      _delay_us( 98 );		/* * 256 = 25ms */			\
      if( --i == 0 ){		/* ... until key >25ms pressed */	\
	flag = 1;		/* set press flag */			\
	i = 1;			/* 1 = key press debounced */		\
	break;								\
      }									\
    }									\
  }									\
  i;				/* return value of Macro */		\
})

#endif

