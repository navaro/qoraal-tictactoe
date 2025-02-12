decl_name       "tic-tac-toe"
decl_version    1

decl_events {
    _tictac_tick
    _tictac_restart
}

statemachine tictactoe {

    startstate ready

    state ready {
        /*
         * Entering the ready state signals readiness (html_ready) to render HTML 
         * when the _html_render event is received.
         */
        enter (html_ready)
        action (_tictac_restart, tictac_restart)
        action (_tictac_tick, tictac_play, [e])
        event (_html_render, html_head)

        /*
         * On receiving _html_render, transition to html_head.
         * The exit action begins the "text/html" response.
         * All subsequent html_emit calls append content to the response.
         */
        exit (html_response, HTML)
    }

    state html {
        enter (html_emit,       "<!DOCTYPE html>\r\n"
                                "<html lang=\"en\">\r\n")
        /*
         * Exiting the html superstate closes the document by emitting the </html> tag.
         */
        exit (html_emit,        "</html>\r\n")
    }

    super html {
        state html_head {
            /*
             * Engine state machine commands are single-line, 
             * but multi-line text blocks are supported. 
             * The closing bracket must be on the same line as the final text line.
             */
            enter (html_emit,   "<head>\r\n"
                                "<meta charset=\"UTF-8\">\r\n"
                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n"
                                "<title>Tic-Tac-Toe</title>\r\n"
                                "<link rel=\"stylesheet\" href=\"/engine/tictaccss\">\r\n")
            event (_state_start, html_board_title)
            exit (html_emit,    "</head>\r\n")
        }
    }

    super html {
        /*
         * States can be nested. You can either nest them directly or repeat the "super" identifier.
         */
        state html_body {
            enter (html_emit,   "<body>\r\n")
            /*
             * html_subst_emit enables token substitution for identifiers in square brackets ([]).
             * Escape brackets if substitution is not intended.
             */
            exit (html_subst_emit,    "<button class=\"restart-btn\" onclick=\"window.location.href='/engine/tictactoe/[_tictac_restart]'\">Restart</button>\r\n"
                                "<button class=\"restart-btn\" onclick=\"window.location.href='/index'\">Take Me Home</button>"
                                "</body>")
        }

        super html_body {
            state html_board_title {
                action (_state_start, html_emit,                        "<h1>Tic-Tac-Toe</h1>\r\n")
                action_ld (_state_start, [a], tictac_status)
                action_eq (_state_start, TICTAC_DRAW,       html_emit,  "<div id=\"winner-message\" class=\"winner\"> Draw </div>\r\n\r\n")
                action_eq (_state_start, TICTAC_PLAYER_WIN, html_emit,  "<div id=\"winner-message\" class=\"winner\"> 👑 Player Wins! 👑 </div>\r\n")
                action_eq (_state_start, TICTAC_AI_WIN,     html_emit,  "<div id=\"winner-message\" class=\"winner\"> 🎉 AI Wins! 🎉 </div>\r\n")
                event (_state_start, html_board_cell)
            }
           
            state html_board {
                /*
                 * Reset the register [r] (cell counter) to zero before rendering the board.
                 */
                enter (r_load, 0)
                enter (html_emit,   "<div class=\"board\">\r\n")
                exit (html_emit,    "</div>\r\n")
            }

            super html_board {
                state html_board_cell {
                    action_ld (_state_start, [a], tictac_cell, [r])

                    /*
                     * If a cell is open, render it as a clickable link triggering a _tictac_tick event.
                     * The cell number is passed via the [e] event register.
                     */
                    action_eq (_state_start, TICTAC_OPEN,   html_subst_emit,"<div class=\"cell\">"
                                                                            "<a href=\"/engine/tictactoe/[_tictac_tick]/[r]\" "
                                                                            "class=\"invisible-link\"></a></div>\r\n")

                    action_eq (_state_start, TICTAC_PLAYER,     html_emit,  "<div class=\"cell x\"></div>\r\n")
                    action_eq (_state_start, TICTAC_AI,         html_emit,  "<div class=\"cell o\"></div>\r\n")
                    action_eq (_state_start, TICTAC_PLAYER_BLINK,   html_emit,  "<div class=\"cell x blink\"></div>\r\n")
                    action_eq (_state_start, TICTAC_AI_BLINK,       html_emit,  "<div class=\"cell o blink\"></div>\r\n")

                    /*
                     * Increment register [r] after processing each cell.
                     * Once [r] reaches 9, set accumulator [a] and return to "ready".
                     * Otherwise, continue rendering the next cell.
                     */
                    action (_state_start, r_inc, 9)
                    event_nt (_state_start, html_board_cell)
                    event_if (_state_start, ready)
                }
            }     
        }
    }
}
