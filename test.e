


decl_name       "tic-tac-toe"
decl_version    1

statemachine html_test {

    startstate ready

    state ready {
        enter (html_ready)
        event (_html_render, html_head)
    }

    // This state emits the doctype + <html> opening
    state html {
        enter (html_emit,       "<!DOCTYPE html>\r\n"
                                "<html lang=\"en\">\r\n")
        exit (html_emit,        "</html>\r\n")
    }

    // Wrap the HEAD + BODY under 'html' superstate
    super html {
        // HEAD
        state html_head {
            enter (html_emit,   "<head>\r\n"
                                "<meta charset=\"UTF-8\">\r\n"
                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n"
                                "<title>Tic-Tac-Toe</title>\r\n"
                                // reference our separate CSS
                                "<link rel=\"stylesheet\" href=\"/engine/css\">"
                                "\r\n"
                                "</head>\r\n")
            exit (html_emit,    "")
            event (_state_start, html_body)
        }
    }

    super html {
        // BODY
        state html_body {
            enter (html_emit,   "<body>\r\n"
                                "<h1>Tic-Tac-Toe</h1>\r\n"
                                "<div class=\"board\">\r\n"
                                "    <div class=\"cell\"><a href=\"/engine/_tick/1\" class=\"invisible-link\"></a></div>\r\n"
                                "    <div class=\"cell\"><a href=\"/engine/_tick/2\" class=\"invisible-link\"></a></div>\r\n"
                                "    <div class=\"cell\"><a href=\"/engine/_tick/3\" class=\"invisible-link\"></a></div>\r\n"
                                "    <div class=\"cell\"><a href=\"/engine/_tick/4\" class=\"invisible-link\"></a></div>\r\n"
                                "    <div class=\"cell x\"></div>\r\n"
                                "    <div class=\"cell \"><a href=\"/engine/_tick/6\" class=\"invisible-link\"></a></div>\r\n"
                                "    <div class=\"cell\"><a href=\"/engine/_tick/7\" class=\"invisible-link\"></a></div>\r\n"
                                "    <div class=\"cell\"><a href=\"/engine/_tick/8\" class=\"invisible-link\"></a></div>\r\n"
                                "    <div class=\"cell\"><a href=\"/engine/_tick/9\" class=\"invisible-link\"></a></div>\r\n"
                                "</div>\r\n"
                                "<button class=\"restart-btn\">Restart</button>\r\n"
                                "<button class=\"restart-btn\" onclick=\"window.location.href='/index'\">Take Me Home</button>\r\n"
                                "</body>")
            exit (html_emit,    "")
            event (_state_start, ready)
        }
    }

}


decl_name       "css"
decl_version    1

statemachine css_test {

    // Start state to emit CSS
    startstate emit_css {
        // If your engine supports something like (set_header, "Content-Type: text/css") do it here
        // or configure it in your route definition externally.

        enter (html_emit,   
            "body {\r\n"
            "    display: flex;\r\n"
            "    flex-direction: column;\r\n"
            "    align-items: center;\r\n"
            "    justify-content: center;\r\n"
            "    height: 100vh;\r\n"
            "    font-family: \"Lucida Sans\", \"Lucida Sans Regular\", \"Lucida Grande\", sans-serif;\r\n"
            "    background: radial-gradient(circle, #1e1e2e 0%, #0c0917 90%);\r\n"
            "    color: #e2e2e2;\r\n"
            "    margin: 0;\r\n"
            "}\r\n"
            "\r\n"
            ".board {\r\n"
            "    display: grid;\r\n"
            "    grid-template-columns: repeat(3, 100px);\r\n"
            "    grid-template-rows: repeat(3, 100px);\r\n"
            "    gap: 0;\r\n"
            "    box-shadow: 0 0 25px rgba(255, 0, 255, 0.5);\r\n"
            "    transition: box-shadow 0.5s ease;\r\n"
            "}\r\n"
            ".board:hover {\r\n"
            "    box-shadow: 0 0 45px rgba(255, 0, 255, 0.9);\r\n"
            "}\r\n"
            "\r\n"
            ".cell {\r\n"
            "    width: 100px;\r\n"
            "    height: 100px;\r\n"
            "    display: flex;\r\n"
            "    align-items: center;\r\n"
            "    justify-content: center;\r\n"
            "    font-size: 2em;\r\n"
            "    font-weight: bold;\r\n"
            "    text-align: center;\r\n"
            "    background-color: transparent;\r\n"
            "    box-sizing: border-box;\r\n"
            "    border-bottom: 6px solid rgba(255, 0, 255, 0.8);\r\n"
            "    border-right: 6px solid rgba(255, 0, 255, 0.8);\r\n"
            "    color: #ffffff;\r\n"
            "    transition: transform 0.3s ease;\r\n"
            "    position: relative;\r\n"
            "}\r\n"
            ".cell:hover {\r\n"
            "    transform: scale(1.1);\r\n"
            "}\r\n"
            "\r\n"
            "/* We'll use pseudo-elements for X or O */\r\n"
            ".cell::before,\r\n"
            ".cell::after {\r\n"
            "    content: \"\";\r\n"
            "    position: absolute;\r\n"
            "    top: 50%;\r\n"
            "    left: 50%;\r\n"
            "    transform: translate(-50%, -50%);\r\n"
            "}\r\n"
            "\r\n"
            ".cell.x::before,\r\n"
            ".cell.x::after {\r\n"
            "    width: 70%;\r\n"
            "    height: 6px;\r\n"
            "    background-color: #fff;\r\n"
            "    border-radius: 4px;\r\n"
            "}\r\n"
            ".cell.x::before {\r\n"
            "    transform: translate(-50%, -50%) rotate(45deg);\r\n"
            "}\r\n"
            ".cell.x::after {\r\n"
            "    transform: translate(-50%, -50%) rotate(-45deg);\r\n"
            "}\r\n"
            "\r\n"
            ".cell.o::before {\r\n"
            "    width: 70%;\r\n"
            "    height: 70%;\r\n"
            "    border: 6px solid #fff;\r\n"
            "    border-radius: 50%;\r\n"
            "}\r\n"
            ".cell.o::after {\r\n"
            "    display: none;\r\n"
            "}\r\n"
            "\r\n"
            "/* Remove borders on last column & row */\r\n"
            ".cell:nth-child(3n) {\r\n"
            "    border-right: none;\r\n"
            "}\r\n"
            ".cell:nth-child(7),\r\n"
            ".cell:nth-child(8),\r\n"
            ".cell:nth-child(9) {\r\n"
            "    border-bottom: none;\r\n"
            "}\r\n"
            "\r\n"
            ".restart-btn {\r\n"
            "    margin-top: 20px;\r\n"
            "    padding: 10px 20px;\r\n"
            "    font-size: 1.2em;\r\n"
            "    cursor: pointer;\r\n"
            "    background-color: #fff;\r\n"
            "    color: #0c0917;\r\n"
            "    border: none;\r\n"
            "    border-radius: 8px;\r\n"
            "    transition: background-color 0.3s ease;\r\n"
            "}\r\n"
            ".restart-btn:hover {\r\n"
            "    background-color: #ececec;\r\n"
            "}\r\n"
            "\r\n"
            "/* Invisible link that fills the entire cell */\r\n"
            ".invisible-link {\r\n"
            "    display: block;\r\n"
            "    width: 100%;\r\n"
            "    height: 100%;\r\n"
            "    text-decoration: none;\r\n"
            "    color: transparent;\r\n"
            "    position: absolute;\r\n"
            "    top: 0;\r\n"
            "    left: 0;\r\n"
            "    z-index: 1;\r\n"
            "}\r\n"
        )
    }
}
