/* rabbit_main_event_loop - main event loop
 * works by running select(), then calling xcb_poll_for_event if the 
 * select() returns data available on the X connection socket, and 
 * rabbit_process_one_event if the returned event is non-null.
 *
 * Other things can also be plugged into the event loop, most immediately,
 * callbacks for reacting to select conditions.
 * 
 *
 * Eventually, this will become a multithread event dispatch system, with
 * events potentially getting dispatched to event loops in other threads. 
 */



/* rabbit_process_one_event - takes an xcb_generic_event_t
 * and does something useful with it, 
 */

/* for the threading stuff, we need an atomic queue construct,
   which can be done lock-free with test-and-set quite easily
   via a retry mechanism */
