#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "qoraal/qoraal.h"
#include "qoraal/svc/svc_services.h"
#include "qoraal/svc/svc_shell.h"



SVC_SHELL_CMD_DECL("tictactoe", qshell_cmd_tictactoe, "");
SVC_SHELL_CMD_DECL("tictactrain", qshell_cmd_tictactrain, "");

#define INPUT_SIZE 9
#define HIDDEN_SIZE 9
#define OUTPUT_SIZE 9

typedef struct {
    double weights_input_hidden[HIDDEN_SIZE][INPUT_SIZE];
    double hidden_biases[HIDDEN_SIZE];
    double weights_hidden_output[OUTPUT_SIZE][HIDDEN_SIZE];
    double output_biases[OUTPUT_SIZE];
} NeuralNet;

NeuralNet _tictactoe_net = {
{
    { 0.106844, 2.297165, -4.215321, 0.590149, -4.113728, 1.123057, -2.406111, 0.652693, 2.410074 },
    { -1.485419, 2.351545, -3.254365, -0.902239, 1.054924, -0.451825, -0.939223, 2.492350, -2.197246 },
    { 0.721562, 0.868623, 2.571977, -3.532937, 0.216167, -3.186169, -2.161942, -4.266016, -1.760417 },
    { 4.915549, 0.471722, -2.216907, 3.307417, -1.075988, 0.506404, 1.420466, 0.498970, -2.845725 },
    { -0.707013, -0.628700, -0.923353, 1.123700, 5.734343, 1.747154, 0.590114, 2.511602, 1.158106 },
    { 1.471237, 0.480728, -3.428099, 3.401812, -1.676710, 3.400047, -1.495710, 3.318829, 2.706812 },
    { 3.480505, 3.960648, 3.179807, -2.151266, 1.626941, -1.576451, -0.142268, 0.399096, -0.765087 },
    { -6.733041, -5.435084, 5.806235, -0.590666, 0.693655, -0.774133, -0.212025, -0.534461, -0.619377 },
    { 5.663660, 1.372808, 0.289790, -0.783382, 4.128768, -2.408603, 0.482060, 0.022660, -4.602510 }
},

{ -1.085286, 2.790615, 3.996572, -0.227859, -5.441354, -0.241621, -3.390699, -0.275622, 2.635717 },

 {
    { 0.736400, 2.059622, 6.050683, -0.965378, -1.117015, -0.062528, -4.617711, -1.298687, 2.435539 },
    { -0.588383, 2.293386, 4.156750, -0.474892, -2.906036, 0.603409, -3.668664, -3.515782, -0.010221 },
    { 1.241191, 2.734489, 4.463278, -0.015362, -3.074955, -0.441200, -1.653899, 7.363835, -0.865544 },
    { 1.127200, -0.962272, -0.273045, 0.690156, -3.969938, -0.305883, 2.244259, -1.192854, -0.043179 },
    { 3.799981, -2.281462, 4.464080, 3.658008, 2.005016, -5.011692, -1.505154, 0.082717, 0.789965 },
    { 1.657756, -2.761479, 0.867956, -0.471131, -2.479380, 0.702748, 3.613657, -1.765321, -1.952891 },
    { 5.723476, 0.786463, 4.733712, 0.615118, -2.254674, -5.613541, -4.248687, 0.032763, 1.037714 },
    { 0.913864, -3.744873, -0.529591, 3.051642, -4.441503, 0.459809, -2.606837, 0.049611, -1.037329 },
    { 1.230620, 0.645857, 3.983960, 4.192639, -0.019080, 0.526311, 2.018243, 0.517314, -8.125889 }
},

 { -0.893318, 1.636943, -0.055843, -1.541960, 0.008355, -0.923598, 2.041277, 0.620558, -0.677074 },

} ;



double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double rand_weight() {
    // Random weight in range [-1, 1]
    return ((double)rand() / RAND_MAX) * 2.0 - 1.0;
}

void init_network(void) {
    NeuralNet *net = &_tictactoe_net ; 
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        for (int j = 0; j < INPUT_SIZE; j++) {
            net->weights_input_hidden[i][j] = rand_weight();
        }
        net->hidden_biases[i] = rand_weight();
    }
    for (int i = 0; i < OUTPUT_SIZE; i++) {
        for (int j = 0; j < HIDDEN_SIZE; j++) {
            net->weights_hidden_output[i][j] = rand_weight();
        }
        net->output_biases[i] = rand_weight();
    }
}

void forward_with_hidden(double input[INPUT_SIZE], double hidden[HIDDEN_SIZE], double output[OUTPUT_SIZE]) {
    NeuralNet *net = &_tictactoe_net ; 
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        double sum = net->hidden_biases[i];
        for (int j = 0; j < INPUT_SIZE; j++) {
            sum += net->weights_input_hidden[i][j] * input[j];
        }
        hidden[i] = sigmoid(sum);
    }
    for (int i = 0; i < OUTPUT_SIZE; i++) {
        double sum = net->output_biases[i];
        for (int j = 0; j < HIDDEN_SIZE; j++) {
            sum += net->weights_hidden_output[i][j] * hidden[j];
        }
        output[i] = sigmoid(sum);
    }
}

void board_to_input(int board[9], double input[9]) {
    for (int i = 0; i < 9; i++) {
        // Mapping: -1 -> 0.0, 0 -> 0.5, 1 -> 1.0
        input[i] = (board[i] + 1) / 2.0;
    }
}

// Returns 1 if player 1 wins, -1 if player -1 wins, 0 if no winner.
int check_winner(int board[9]) {
    int wins[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8},
        {0,3,6}, {1,4,7}, {2,5,8},
        {0,4,8}, {2,4,6}
    };
    for (int i = 0; i < 8; i++) {
        int a = wins[i][0], b = wins[i][1], c = wins[i][2];
        if (board[a] != 0 && board[a] == board[b] && board[b] == board[c])
            return board[a];
    }
    return 0;
}

int board_full(int board[9]) {
    for (int i = 0; i < 9; i++) {
        if (board[i] == 0)
            return 0;
    }
    return 1;
}

// Epsilon-greedy move selection.
int choose_move_epsilon(int board[9], double epsilon) {
    NeuralNet *net = &_tictactoe_net ; 
    int legal_moves[9], count = 0;
    for (int i = 0; i < 9; i++) {
        if (board[i] == 0)
            legal_moves[count++] = i;
    }
    if (count == 0)
        return -1; // No moves available.

    double r = (double)rand() / RAND_MAX;
    if (r < epsilon) {
        // Explore: random legal move.
        return legal_moves[rand() % count];
    } else {
        // Exploit: choose move with highest network output.
        double input[INPUT_SIZE], hidden[HIDDEN_SIZE], output[OUTPUT_SIZE];
        board_to_input(board, input);
        forward_with_hidden(input, hidden, output);
        int best_move = legal_moves[0];
        double best_value = -1.0;
        for (int i = 0; i < count; i++) {
            int move = legal_moves[i];
            if (output[move] > best_value) {
                best_value = output[move];
                best_move = move;
            }
        }
        return best_move;
    }
}

// Q-learning training step for the chosen action.
void train_step(int board[9], int action, double reward, int next_board[9], int terminal, double learning_rate, double gamma) {
    NeuralNet *net = &_tictactoe_net ; 
    double input[INPUT_SIZE];
    board_to_input(board, input);
    double hidden[HIDDEN_SIZE], output[OUTPUT_SIZE];
    forward_with_hidden(input, hidden, output);

    double q_sa = output[action];
    double target;
    if (terminal) {
        target = reward;
    } else {
        double next_input[INPUT_SIZE], next_hidden[HIDDEN_SIZE], next_output[OUTPUT_SIZE];
        board_to_input(next_board, next_input);
        forward_with_hidden(next_input, next_hidden, next_output);
        // Consider only legal moves for next state.
        double max_next = 0.0;
        int first = 1;
        for (int i = 0; i < 9; i++) {
            if (next_board[i] == 0) {
                if (first) {
                    max_next = next_output[i];
                    first = 0;
                } else if (next_output[i] > max_next) {
                    max_next = next_output[i];
                }
            }
        }
        target = reward + gamma * max_next;
    }

    // Compute error and update only for the chosen action.
    double error = target - q_sa;
    double output_deriv = q_sa * (1 - q_sa);
    double delta_output = error * output_deriv;

    // Update weights for hidden->output (only for the action taken).
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        net->weights_hidden_output[action][i] += learning_rate * delta_output * hidden[i];
    }
    net->output_biases[action] += learning_rate * delta_output;

    // Backpropagate into hidden layer and update input->hidden weights.
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        double error_hidden = delta_output * net->weights_hidden_output[action][i];
        double hidden_deriv = hidden[i] * (1 - hidden[i]);
        double delta_hidden = error_hidden * hidden_deriv;
        for (int j = 0; j < INPUT_SIZE; j++) {
            net->weights_input_hidden[i][j] += learning_rate * delta_hidden * input[j];
        }
        net->hidden_biases[i] += learning_rate * delta_hidden;
    }
}

void copy_board(int dest[9], int src[9]) {
    memcpy(dest, src, sizeof(int) * 9);
}

void print_board(int board[9]) {
    // Mapping: -1 -> 'O', 0 -> ' ', 1 -> 'X'
    char symbols[3] = { 'O', ' ', 'X' };
    for (int i = 0; i < 9; i++) {
        int idx = board[i] + 1;  // -1 becomes 0, 0 becomes 1, 1 becomes 2
        printf(" %c ", symbols[idx]);
        if ((i + 1) % 3 == 0) {
            printf("\n");
            if (i != 8)
                printf("---+---+---\n");
        } else {
            printf("|");
        }
    }
}

void print_trained_weights(void) {
    NeuralNet *net = &_tictactoe_net ; 
    int i, j;
    printf("\n// Trained Weights for Initialization\n\n");

    // Print weights_input_hidden
    printf("double weights_input_hidden[%d][%d] = {\n", HIDDEN_SIZE, INPUT_SIZE);
    for (i = 0; i < HIDDEN_SIZE; i++) {
        printf("    { ");
        for (j = 0; j < INPUT_SIZE; j++) {
            printf("%.6f", net->weights_input_hidden[i][j]);
            if (j < INPUT_SIZE - 1)
                printf(", ");
        }
        printf(" }");
        if (i < HIDDEN_SIZE - 1)
            printf(",\n");
        else
            printf("\n");
    }
    printf("};\n\n");

    // Print hidden_biases
    printf("double hidden_biases[%d] = { ", HIDDEN_SIZE);
    for (i = 0; i < HIDDEN_SIZE; i++) {
        printf("%.6f", net->hidden_biases[i]);
        if (i < HIDDEN_SIZE - 1)
            printf(", ");
    }
    printf(" };\n\n");

    // Print weights_hidden_output
    printf("double weights_hidden_output[%d][%d] = {\n", OUTPUT_SIZE, HIDDEN_SIZE);
    for (i = 0; i < OUTPUT_SIZE; i++) {
        printf("    { ");
        for (j = 0; j < HIDDEN_SIZE; j++) {
            printf("%.6f", net->weights_hidden_output[i][j]);
            if (j < HIDDEN_SIZE - 1)
                printf(", ");
        }
        printf(" }");
        if (i < OUTPUT_SIZE - 1)
            printf(",\n");
        else
            printf("\n");
    }
    printf("};\n\n");

    // Print output_biases
    printf("double output_biases[%d] = { ", OUTPUT_SIZE);
    for (i = 0; i < OUTPUT_SIZE; i++) {
        printf("%.6f", net->output_biases[i]);
        if (i < OUTPUT_SIZE - 1)
            printf(", ");
    }
    printf(" };\n\n");
}


static int32_t
qshell_cmd_tictactoe (SVC_SHELL_IF_T * pif, char** argv, int argc)
{
    // Demonstration game (AI vs. random opponent, no exploration).
    printf("\n=== Demonstration Game ===\n");
    int board[9] = {0,0,0, 0,0,0, 0,0,0};
    int current_player = 0;
    while (!check_winner(board) && !board_full(board)) {
        if (current_player == 1) {
            int action = choose_move_epsilon(board, 0.0);
            if (action == -1)
                break;
            board[action] = 1;
            printf("\nAI moves at position %d:\n", action);
            print_board(board);
            current_player = -1;
        } else {
            int legal_moves[9], count = 0;
            for (int i = 0; i < 9; i++) {
                if (board[i] == 0)
                    legal_moves[count++] = i;
            }
            if (count == 0)
                break;
            int move = legal_moves[rand() % count];
            board[move] = -1;
            printf("\nOpponent moves at position %d:\n", move);
            print_board(board);
            current_player = 1;
        }
    }
    int winner = check_winner(board);
    if (winner == 1)
        printf("\nAI wins the demonstration game!\n");
    else if (winner == -1)
        printf("\nOpponent wins the demonstration game!\n");
    else
        printf("\nIt's a draw in the demonstration game!\n");

    return EOK ;
}

static int32_t
qshell_cmd_tictactrain (SVC_SHELL_IF_T * pif, char** argv, int argc)
{
    srand(time(NULL));

    init_network();

    // Training hyperparameters
    int episodes = 200000;
    double learning_rate = 0.1;
    double gamma = 0.9;
    double epsilon = 0.3;  // initial exploration rate

    // Training loop: AI plays against a random opponent.
    for (int ep = 0; ep < episodes; ep++) {
        // Randomly assign AI role: 1 (first mover) or -1 (second mover)
        int ai_player = (rand() % 2 == 0) ? 1 : -1;
        int board[9] = {0,0,0, 0,0,0, 0,0,0};
        int game_over = 0;
        int current_player = 1;  // game always starts with player 1
        int last_ai_board[9];
        int last_ai_action = -1;
        int ai_waiting_update = 0;

        while (!game_over) {
            if (current_player == ai_player) {
                // AI's turn.
                copy_board(last_ai_board, board);
                int action = choose_move_epsilon(board, epsilon);
                if (action == -1)
                    break;
                board[action] = ai_player;

                int winner = check_winner(board);
                if (winner == ai_player) {
                    // AI wins: reward +1.
                    train_step(last_ai_board, action, 1.0, board, 1, learning_rate, gamma);
                    game_over = 1;
                } else if (board_full(board)) {
                    // Draw: reward 0.5.
                    train_step(last_ai_board, action, 0.5, board, 1, learning_rate, gamma);
                    game_over = 1;
                } else {
                    last_ai_action = action;
                    ai_waiting_update = 1;
                    current_player = -current_player;
                }
            } else {
                // Opponent's turn (random move).
                int legal_moves[9], count = 0;
                for (int i = 0; i < 9; i++) {
                    if (board[i] == 0)
                        legal_moves[count++] = i;
                }
                if (count == 0)
                    break;
                int move = legal_moves[rand() % count];
                board[move] = -ai_player; // Opponent uses the opposite marker.

                int winner = check_winner(board);
                if (winner == -ai_player) {
                    // Opponent wins: AI loses (reward -1).
                    if (ai_waiting_update)
                        train_step(last_ai_board, last_ai_action, -1.0, board, 1, learning_rate, gamma);
                    game_over = 1;
                } else if (board_full(board)) {
                    // Draw: reward 0.5.
                    if (ai_waiting_update)
                        train_step(last_ai_board, last_ai_action, 0.5, board, 1, learning_rate, gamma);
                    game_over = 1;
                } else {
                    if (ai_waiting_update) {
                        train_step(last_ai_board, last_ai_action, 0.0, board, 0, learning_rate, gamma);
                        ai_waiting_update = 0;
                    }
                    current_player = -current_player;
                }
            }
        }
        // Decay epsilon gradually.
        if (epsilon > 0.05)
            epsilon *= 0.9999;
        if ((ep + 1) % 5000 == 0)
            printf("Episode %d completed. Epsilon: %.3f\n", ep + 1, epsilon);
    }


    // Print out the trained weights and biases as C initialization arrays.
    print_trained_weights();

    return 0;
}
