decl_name       "tic-tac-toe"
decl_version    1

statemachine html_test {

    startstate ready

    state ready {
        enter (html_ready)
        event (_html_render, html_head)
    }

    state html {
        enter (html_emit,       "<!DOCTYPE html>\r\n"
                                "<html lang=\"en\">\r\n")
        exit (html_emit,        "</html>\r\n")
    }

    super html {
        state html_head {
            enter (html_emit,   "<head>\r\n"
                                "<meta charset=\"UTF-8\">\r\n"
                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n"
                                "<title>Tic-Tac-Toe</title>\r\n"
                                "<style>\r\n"
                                "    body {\r\n"
                                "        display: flex;\r\n"
                                "        flex-direction: column;\r\n"
                                "        align-items: center;\r\n"
                                "        justify-content: center;\r\n"
                                "        height: 100vh;\r\n"
                                "        font-family: \"Lucida Sans\", \"Lucida Sans Regular\", \"Lucida Grande\", sans-serif;\r\n"
                                "        /* Slightly spooky, eerie gradient background */\r\n"
                                "        background: radial-gradient(circle, #1e1e2e 0%, #0c0917 90%);\r\n"
                                "        color: #e2e2e2;\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    .board {\r\n"
                                "        display: grid;\r\n"
                                "        grid-template-columns: repeat(3, 100px);\r\n"
                                "        grid-template-rows: repeat(3, 100px);\r\n"
                                "        gap: 0;\r\n"
                                "        /* A faint glow surrounding the board */\r\n"
                                "        box-shadow: 0 0 25px rgba(255, 0, 255, 0.5);\r\n"
                                "        transition: box-shadow 0.5s ease;\r\n"
                                "    }\r\n"
                                "    .board:hover {\r\n"
                                "        /* Subtle pulsing glow when hovered */\r\n"
                                "        box-shadow: 0 0 45px rgba(255, 0, 255, 0.9);\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    .cell {\r\n"
                                "        width: 100px;\r\n"
                                "        height: 100px;\r\n"
                                "        display: flex;\r\n"
                                "        align-items: center;\r\n"
                                "        justify-content: center;\r\n"
                                "        font-size: 2em;\r\n"
                                "        font-weight: bold;\r\n"
                                "        text-align: center;\r\n"
                                "        /* Make the cell color match the background, for a floating effect */\r\n"
                                "        background-color: transparent;\r\n"
                                "        box-sizing: border-box;\r\n"
                                "        border-bottom: 6px solid rgba(255, 0, 255, 0.8); /* Thicker purple/pink border */\r\n"
                                "        border-right: 6px solid rgba(255, 0, 255, 0.8);\r\n"
                                "        color: #ffffff;\r\n"
                                "        transition: transform 0.3s ease;\r\n"
                                "        position: relative; /* For pseudo-elements in specialized cells */\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    /* Slight hover effect on individual cells */\r\n"
                                "    .cell:hover {\r\n"
                                "        transform: scale(1.1);\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    /* Base pseudo-element styles for BOTH X and O */\r\n"
                                "    .cell::before,\r\n"
                                "    .cell::after {\r\n"
                                "        content: \"\";\r\n"
                                "        position: absolute;\r\n"
                                "        top: 50%;\r\n"
                                "        left: 50%;\r\n"
                                "        transform: translate(-50%, -50%);\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    /* X overrides (two crossed lines) */\r\n"
                                "    .cell.x::before,\r\n"
                                "    .cell.x::after {\r\n"
                                "        width: 70%;\r\n"
                                "        height: 6px;\r\n"
                                "        background-color: #fff;\r\n"
                                "        border-radius: 4px;\r\n"
                                "    }\r\n"
                                "    .cell.x::before {\r\n"
                                "        transform: translate(-50%, -50%) rotate(45deg);\r\n"
                                "    }\r\n"
                                "    .cell.x::after {\r\n"
                                "        transform: translate(-50%, -50%) rotate(-45deg);\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    /* O overrides (one ring) */\r\n"
                                "    .cell.o::before {\r\n"
                                "        width: 70%;\r\n"
                                "        height: 70%;\r\n"
                                "        border: 6px solid #fff;\r\n"
                                "        border-radius: 50%;\r\n"
                                "    }\r\n"
                                "    .cell.o::after {\r\n"
                                "        display: none; /* not needed for O */\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    /* Remove the right border on the last column */\r\n"
                                "    .cell:nth-child(3n) {\r\n"
                                "        border-right: none;\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    /* Remove the bottom border on the last row */\r\n"
                                "    .cell:nth-child(7),\r\n"
                                "    .cell:nth-child(8),\r\n"
                                "    .cell:nth-child(9) {\r\n"
                                "        border-bottom: none;\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    .restart-btn {\r\n"
                                "        margin-top: 20px;\r\n"
                                "        padding: 10px 20px;\r\n"
                                "        font-size: 1.2em;\r\n"
                                "        cursor: pointer;\r\n"
                                "        background-color: #fff;\r\n"
                                "        color: #0c0917;\r\n"
                                "        border: none;\r\n"
                                "        border-radius: 8px;\r\n"
                                "        transition: background-color 0.3s ease;\r\n"
                                "    }\r\n"
                                "\r\n"
                                "    .restart-btn:hover {\r\n"
                                "        background-color: #ececec;\r\n"
                                "    }\r\n"
                                "</style>\r\n")
            exit (html_emit,    "</head>\r\n")
            event (_state_start, html_body)
        }
    }

    super html {
        state html_body {
            enter (html_emit,   "<body>\r\n"
                                "<h1>Tic-Tac-Toe</h1>\r\n"
                                "<div class=\"board\">\r\n"
                                "    <div class=\"cell\"></div>\r\n"
                                "    <div class=\"cell\"></div>\r\n"
                                "    <div class=\"cell\"></div>\r\n"
                                "    <div class=\"cell\"></div>\r\n"
                                "    <div class=\"cell x\"></div>\r\n"
                                "    <div class=\"cell \"></div>\r\n"
                                "    <div class=\"cell\"></div>\r\n"
                                "    <div class=\"cell\"></div>\r\n"
                                "    <div class=\"cell\"></div>\r\n"
                                "</div>\r\n"
                                "<button class=\"restart-btn\">Restart</button>\r\n")
            exit (html_emit,    "</body>")
            event (_state_start, ready)
        }
    }

}
