<div align="center">

# Qoraal Tic-Tac-Toe

<div align="center">


### This is what it all was for. Third door on the left, if you dare...


<div align="center">



<br>

</div>

<div align="center">
  <img src="tictactoe.png" alt="Welcome" />
</div>
<br>
<div align="center">

#### Check this out first—then hit me.



<div align="left">


```cpp
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
         * Entering the ready state we signal that we are ready (html_ready) to render 
         * html should we receive the _html_render event.
         */
        enter (html_ready)
        action (_tictac_restart, tictac_restart)
        action (_tictac_tick, tictac_play, [e])
        event (_html_render, html_head)
        /*
         * The state exit action here wil begin the "text/html" response. All subsequent
         * html_emit calls will add to the response content.
         */
        exit (html_response, HTML)

    }

    state html {
        enter (html_emit,       "<!DOCTYPE html>\r\n"
                                "<html lang=\"en\">\r\n")
        /*
         * When we exit the html super state, to return to the "ready" state again, the 
         * exit action will close the HTML document by emitting the closing </html> tag.
         */
        exit (html_emit,        "</html>\r\n")

    }

    super html {
        state html_head {
            /*
             * Engine machine language commands are single-line. However, multi-line text blocks are
             * allowed. Ensure the closing bracket is on the same line as the final line of text.
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
             * Use html_subst_emit to perform token substitution on identifiers in square brackets ([]).
             * Escape brackets if you don’t intend to substitute.
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
                 * Reset the register [r], used as cell counter, to zero before rendering the board.
                 */
                enter (r_load, 0)
                enter (html_emit,   "<div class=\"board\">\r\n")
                exit (html_emit,    "</div>\r\n")

            }

            super html_board {

                state html_board_cell {
                    action_ld (_state_start, [a], tictac_cell, [r])
                    /*
                     * If the cell is still open, create a clickable link that triggers a _tictac_tick event.
                     * The cell number is passed in the event register [e].
                     */
                    action_eq (_state_start, TICTAC_OPEN,   html_subst_emit,"<div class=\"cell\">"
                                                                            "<a href=\"/engine/tictactoe/[_tictac_tick]/[r]\" "
                                                                            "class=\"invisible-link\"></a></div>\r\n")
                    action_eq (_state_start, TICTAC_PLAYER,     html_emit,  "<div class=\"cell x\"></div>\r\n")
                    action_eq (_state_start, TICTAC_AI,         html_emit,  "<div class=\"cell o\"></div>\r\n")
                    action_eq (_state_start, TICTAC_PLAYER_BLINK,   html_emit,  "<div class=\"cell x blink\"></div>\r\n")
                    action_eq (_state_start, TICTAC_AI_BLINK,       html_emit,  "<div class=\"cell o blink\"></div>\r\n")
                    /*
                     * After processing a cell, the register [r] increments until it reaches 9.
                     * Once [r] hits 9, the [a] accumulator is set, and we return to the "ready" state (event_if).
                     * Otherwise, we continue rendering the next cell (event_nt).
                     */
                    action (_state_start, r_inc, 9)
                    event_nt (_state_start, html_board_cell)
                    event_if (_state_start, ready)

                }

            }     

        }

    }

}
```


### Lets get into it
At its core, a hierarchical state machine can be powerful tool for structuring logic in a modular, maintainable way. When you use it to render structured text like HTML, you unlock flexible method for building dynamic web applications.

The __Qoraal Engine__ framework leverages this concept by mapping state transitions to HTML rendering, it turns what could be a tangle of code into a clear, hierarchical process. This not only keeps your code neat and scalable but also makes development easy, as every state tells a part of the application's story.

__Engine__ goes even further by integrating backend logic, as demonstrated in the tic-tac-toe example. Functions `tictac_play`, `tictac_status`, and `tictac_cell` connect with the AI game backend, handling moves, checking game status, while `html_ready`, `html_response` and `html_emit` update the board rendering. The beauty of __Engine__ is in how it unifies these components: the state machine manages the HTML output and user interactions, while the backend logic processes the game mechanics, resulting in a cohesive and dynamic experience.



## Quick Start  

The demo application can be compiled using the **POSIX port**, allowing you to evaluate it directly on your PC!  

For embedded targets, currently supported **RTOS options** include **ChibiOS, FreeRTOS, and ThreadX**, provided you have an IP stack like **LwIP**.  

⚠️ **Note:** If running in **GitHub Codespaces**, the application will use **port forwarding**, and Codespaces will provide a link to access the web interface from your browser on port 8080.

### Running on Windows/Linux/Codespace  

1. Open your development environment and **checkout the repository**.  
2. Run the appropriate script based on your OS:  
   - **Windows**:  
     ```sh
     > build_and_run.bat
     ```  
   - **Linux/Codespcace**:  
     ```sh
     $ sh ./build_and_run.sh
     ```  
3. When the application starts, a shell will open, displaying **startup logs**.  
4. If building locally, you can access the web interface at:  
   - **http://127.0.0.1:8080** (for local builds)  
   - Use **your build machine’s IP** if running remotely.
   - In a codespace, click on the link in your browser.

That’s it—you're up and running! 🚀  


On system startup, the Tic-Tac AI initializes with a model trained over 200,000 iterations. The initial model parameters were precomputed and stored to ensure immediate availability at launch.

:bulb: Tip: You can retrain your AI using the console command `tictactrain x`, where `x` is the number of iterations you want to train it for.

So dive in and experience how structured state machines can transform your approach to rendering, interactivity, and even game logic!

