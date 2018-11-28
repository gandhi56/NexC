
#include "../include/player.h"


Player::Player(){
	init_vars();
}

Player::Player(char* servIp, int port){
	/*
	 * Constructor: initialize player sockets
	 * and create a game instance once the
	 * necessary game settings are read from
	 * the connected server.
	 */
	
	// initialize member variables
	uint numRows;
	uint numColumns;	
	char buffer[40];
	
	init_vars();
	attach_socket(servIp, port);

	// read game settings
	if (socketConnected){
		memset(buffer, 0, sizeof(buffer));
		read(_socket.clientSd, (char*)&buffer, 
			sizeof(buffer));
		cout << "rec:" << buffer << endl;
		send(_socket.clientSd, "ok", strlen("ok"), 0);
		
		read_settings(buffer, numRows, numColumns);
		
		// create state instance
		gameState.set_size(numRows, numColumns);
		gameState.create_graph();

		// set initial position of gameState
		memset(&buffer, 0, sizeof(buffer));
		read(_socket.clientSd, (char *)&buffer, 
			sizeof(buffer));
		set_state(string(buffer));
		send(_socket.clientSd, "ok", strlen("ok"), 0);
	}
}

Player::~Player(){
	/*
		Destructor
	*/
	
	if (socketConnected)
		close(this->_socket.clientSd);
}

void Player::init_vars(){
	movesCount = 0;
	socketConnected = false;
}

void Player::attach_socket(char* servIp, int port){
	
	// setup client-side socket
	_socket.port = port;
	_socket.host = gethostbyname(servIp);
	_socket.clientSd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char*)&_socket.sendSockAddr, 
		sizeof(_socket.sendSockAddr));
	_socket.sendSockAddr.sin_family = AF_INET;
	_socket.sendSockAddr.sin_addr.s_addr = 
		inet_addr(servIp);

	// connect to port
	_socket.sendSockAddr.sin_port = htons(port);
	int status = connect(_socket.clientSd, 
		(sockaddr*)&_socket.sendSockAddr, 
		sizeof(_socket.sendSockAddr));
	if (status < 0){
		cout << "Error connecting to _socket!" << endl;
	}
	cout << "Connected to the server!" << endl;
	
	socketConnected = true;
}

void Player::set_state(string moves){
    uint i = 0;
    uint j = 1;
	if(moves == ";") return;
    while (i < moves.length()){
        if (moves[i]=='B'||moves[i]=='W'||moves[i]=='?'){
            j = i + 1;
            while (moves[j] != ';') j++;
	        gameState.update(moves.substr(i, j-i));
            i = j+1;
        }
		else{
			cout << "Error parsing start state" << endl;
			break;
		}
    }
}

void Player::read_settings(char* buff, uint& rows, uint& cols){	
	/*
		read row and column dimensions from server.
	*/
	
	string _buffer = string(buff);
	uint i = 0;
	while (_buffer[i] != 'r')	i++;
	i++;
	if (_buffer[i+1] == '-'){
		rows = stoi(_buffer.substr(i, 1));
	}
	else{
		rows = stoi(_buffer.substr(i, 2));
	}

	i = 0;
	while (_buffer[i] != 'c')	i++;
	i++;
	if (_buffer[i+1] == '-'){
		cols = stoi(_buffer.substr(i, 1));
	}
	else{
		cols = stoi(_buffer.substr(i, 2));
	}

	myStone = _buffer[_buffer.length() - 2];
	cout << "rows=" << rows << endl;
	cout << "cols=" << cols << endl;
	cout << "stone=" << myStone << endl;
	
}



// -------------------------------------------------------------------
vector<string> Player::get_moves(State state, bool isMax){
	string myStone = (isMax?"B":"W");

	// store all cells where a stone may be placed
	vector<string> emptyPos;
	vector<string> stonePos;
	vector<string> neutralPos;
	for (uint row = 0; row < state.numRows; ++row){
		for (uint col = 0; col < state.numColumns; ++col){
			uint key = row * state.numRows + col;
			if (state.graph[key].first == EMPTY){
				emptyPos.push_back(state.get_key(row, col));
			}
			else if((state.graph[key].first==BLACK and isMax) or
				(state.graph[key].first == WHITE and !isMax)){
				stonePos.push_back(state.get_key(row, col));
			}
			else if (state.graph[key].first == NEUTRAL){
				neutralPos.push_back(state.get_key(row, col));
			}
		}
	}

	vector<string> moves;

	if (emptyPos.size() >= 2){
	
		for (uint i = 0; i < emptyPos.size(); ++i){
			for (uint j = i+1; j < emptyPos.size(); ++j){
				string key0 = myStone;
				key0 += emptyPos[i] + "?" + emptyPos[j];
				moves.push_back(key0);

				string key1 = "?";
				key1 += emptyPos[i] + myStone  + emptyPos[j];
				moves.push_back(key1);
			}
		}
	}

	if (neutralPos.size() >= 2 && stonePos.size() >= 1){
				
		for (uint i = 0; i < neutralPos.size(); ++i){
			for (uint j = i + 1; j < neutralPos.size(); ++j){
				for (uint k = 0; k < stonePos.size(); ++k){
					string key = myStone;
					key += neutralPos[i]+myStone+neutralPos[j];
					key += "?" + stonePos[k];
					moves.push_back(key);
				}
			}
		}
	}


	return moves;
}

void Player::solve(State state, bool isMax, bool disp){
	int alpha = -100;
	int beta = 100;
	int value = negamax(state, 100, isMax, alpha, beta, disp);
	cout << "alpha=" << alpha << endl;
	cout << "beta="  << beta << endl;
	cout << "value=" << value << endl;
}

int Player::evaluate(State state, bool isMax){
	char gameStatus = state.status();
	if (gameStatus == GAME_NOT_OVER)	return -100;
	
	int value;
	if (gameStatus == BLACK_WIN)		value = 1;
	else if (gameStatus == WHITE_WIN)	value = -1;
	else if (gameStatus == DRAW)		value = 0;
	if (!isMax)	value *= -1;
	return value;
}

string Player::best_neg_move(State state, int depth, bool isMax, bool disp){
	
	if (disp)	state.show();

	char play0 = (char)(isMax?'B':'W');	// player to move
	vector<string> moves = get_moves(state, isMax);
	int alpha = -100;
	int beta = 100;
	int value = -100;
	string bestMove = moves[0];
	for (string move : moves){
		if (disp)	cout << "playing " << move << endl;
		state.update(move);		// play0 plays a move
		int negVal = 
			-negamax(state, depth-1, !isMax, -beta, -alpha, disp);
		if (negVal > value){
			value = negVal;
			bestMove = move;
		}

		alpha = max(alpha, value);
		if (alpha >= beta)	break;

		state.revert(move, play0);
	}

	return bestMove;

}

int Player::negamax(State state, int depth, bool isMax, int alpha, int beta, bool disp){
	
	if (disp)	state.show();

	char play0 = (char)(isMax?'B':'W');	// player to move
	int value = evaluate(state, isMax);

	// return value if game over
	if (value != -100 or !depth)	return value;

	vector<string> moves = get_moves(state, isMax);
	if (moves.size() == 0){
		return 0;	// return DRAW
	}
	
	for (string move : moves){
		if (disp)	cout << "playing " << move << endl;
		state.update(move);
		value = max(value, -negamax(state, depth-1, !isMax, 
			-beta, -alpha, disp));	
		alpha = max(alpha, value);
		if (alpha >= beta)	break;	// alpha-beta cutoff
		state.revert(move, play0);
	}
	return value;
}

// -------------------------------------------------------------------
void Player::run(bool disp){
	/*
		Run game over server.
	*/

	string data;
	char _data[40];
	while (1){
		memset(_data, 0, sizeof(_data));
		read(this->_socket.clientSd, 
			(char*)&_data, sizeof(_data));
		cout << "rec:" << _data << endl;

		if (!strcmp(_data, "!")){
			exit(1);
		}
		else if (!strcmp(_data, "Move?")){

			// send move here
			//data =best_move(gameState, myStone, 0);
			data = best_neg_move(gameState, 100, myStone == 'B', disp);

			cout << "sending " << data.c_str() << endl;
			send(this->_socket.clientSd, data.c_str(), 
				strlen(data.c_str()), 0);
		}
		else if (!strcmp(_data, "+")){
			cout << "Yahoo, I won!!" << endl;
			break;
		}
		else if (!strcmp(_data, "-")){
			cout << "Ugh, I lost!" << endl;
			break;
		}
		else if (!strcmp(_data, "#")){
			cout << "Draw!" << endl;
			break;
		}
		else if(_data[0] == '>'){
			// to update state in memory
			data = string(_data);
			string move = data.substr(1, data.length() - 1);
			gameState.update(move);
			movesCount++;
			send(this->_socket.clientSd, "ok", 
				strlen("ok"), 0);
			continue;
			
		}
		else{
			cout << "breaking" << endl;
			break;
		}

	}
}

