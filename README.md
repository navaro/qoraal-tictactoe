<div align="center">

# Qoraal Tic-Tac-Toe


Dynamic HTML generation powered by hierarchical state machines.

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://github.com/codespaces/new?hide_repo_select=true&repo=navaro/qoraal-tictactoe)

![TicTacToe](tictactoe.png)

</div>


## Why this is different
- **The page is rendered by a hierarchical state machine** (not templates, not string soup).
- **Game logic + AI live in normal C functions**.
- **HTTP requests trigger events**, and transitions **emit structured HTML**.
- **State machines are compiled from the engine language into a compact runtime form** (tiny footprint).

## Quick Start (POSIX -- Linux / Windows / Codespaces)
This demo compiles via the POSIX port, so you can run it locally or in a GitHub Codespace.

### 1) Build
```bash
make all
```

### 2) Run + open the web UI
Tha game will automatically start, you will see the following log:  
`WSERV : : web server running on port 8080 without SSL!!`

- **Codespaces:** GitHub will offer a forwarded-port link (check the **PORTS** tab).
- **Local:** open `http://127.0.0.1:8080`

Thats it.

## What you will see
A web UI served on **port 8080** where:

- you play tic-tac-toe,
- the AI responds,
- and the entire HTML response is built by the HSM via `html_emit()` / `html_subst_emit()` actions.

---


## Platforms

### Verified
- POSIX (Linux/macOS) — built/tested via `make all`
- Zephyr RTOS (nRF etc.)
- FreeRTOS
- ThreadX
- ChibiOS (legacy)

> If you run this on a target and it works, drop the board/RTOS + toolchain in an issue/PR and I’ll add it here.

## Networking / IP stacks

### POSIX
- OS TCP/IP stack (BSD sockets)

### Embedded (depending on platform)
- Zephyr networking stack
- ThreadX NetX / NetX Duo (BSD Socket API)
- FreeRTOS (LWIP or depending on your port)

---

## What is different 

### HSMs map cleanly to HTML structure
- **Superstates** behave like HTML containers (document → head/body → board → cell).
- **Enter/exit actions** guarantee tags open/close correctly.
- **Events** drive the render lifecycle (request → transition → emit → response).

This approach scales weirdly well for embedded systems where you want:
- deterministic output,
- tight control over allocations,
- and a structure that doesnt devolve into spaghetti.

---

## How it works

### The request → render loop
1. An HTTP request hits the Qoraal HTTP handler.
2. That triggers an event like `_html_render` / `_tictac_tick`.
3. The state machine transitions through render states.
4. Each state emits a chunk of HTML.
5. The response is sent to the browser.

### Game logic stays normal
The HSM orchestrates; the game functions do the work:
- `tictac_play` – apply move, decide AI response
- `tictac_status` – win/draw/ongoing
- `tictac_cell` – per-cell state (open / player / AI / blink)

---
## AI (yes, really)
On startup the AI loads a model trained over **200,000 iterations** so it’s ready instantly.

You can retrain it from the console:
```text
tictactrain x
```
Where `x` is the number of training iterations.

---

<div align="center">
  
## Now, check this out.

> A demo using the Qoraal Engine to generate HTML via hierarchical state machines, showcasing structured rendering + AI-driven game logic.

</div>


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
             * Engine Machine Language commands are single-line, 
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

                    /*
                     * Load the state of the cell [r] into the accumulator [a].
                     */
                    action_ld (_state_start, [a], tictac_cell, [r])

                    /*
                     * The action_eq compares the accumulator to the state each cell can be in. If equal, 
                     * the given html is emmited.
                     *
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
                     * Once [r] reaches 9, the accumulator [a] is set and the event transition to "ready".
                     * Otherwise, the event transition back to "html_board_cell" rendering the next cell.
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



:bulb: Tip: You can retrain your AI using the console command `tictactrain x`, where `x` is the number of iterations you want to train it for.

So dive in and experience how structured state machines can transform your approach to rendering, interactivity, and even game logic!



## Related projects
- Qoraal HTTP: https://github.com/navaro/qoraal-http  
- Qoraal Engine: https://github.com/navaro/qoraal-engine

---

## License
MIT — see `LICENSE`.




