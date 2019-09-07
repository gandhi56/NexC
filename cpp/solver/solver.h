#ifndef _SOLVER_
#define _SOLVER_

#define MAX_ITERATIONS 1000

#define SHUFFLE_MOVES

#include <algorithm>	// for random shuffle

class Action{
public:
	Action& operator=(const Action& a);
};

class State{
public:
	State& operator=(State& s);
	void show();
	bool update(Action action);
	void revert(Action& action);
	void get_moves(...);
	char check_win();
	char player1();
	char player2();
	char draw();
	int evaluate(bool isMax);
};

template <class state_t, class action_t>
class Solver{
public:
	state_t state;
	action_t bestAction;
	Solver(){
		cout << "Solver says Hi!" << endl;
	}
	void set_state(state_t s){
		state = s;
	}

	// alpha beta negamax ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	action_t solve(){
		return action_t("");
	}
	int negamax(state_t s, int alpha, int beta, bool isMax, int depth){
		// cout << "negmax call " << depth << endl;
		int value = s.evaluate(isMax);
		// cout << "value = " << value << endl;
		if (value != -100 and value != 100){
			return value;
		}

		vector<action_t> actions;
		s.get_moves(actions, isMax);

		#ifdef SHUFFLE_MOVES
		srand(time(nullptr));
		random_shuffle(actions.begin(), actions.end());
		#endif

		for (auto action : actions){
			s.update(action);
			if (depth == 0){
				int neg = -negamax(s, -beta, -alpha, !isMax, depth+1);
				if (neg > value){
					value = neg;
					bestAction = action;
				}
			}
			else{
				value = max(value, -negamax(s, -beta, -alpha, !isMax, depth+1));
			}
			alpha = max(alpha, value);
			if (alpha >= beta)
				break;
			s.revert(action);
		}

		if (depth == 0)
			cout << "best move = " << bestAction.move << endl;

		return value;
	}
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	~Solver(){
		cout << "Solver dies" << endl;
	}
};



// template<class state_t, class action_t>
// class MCTNode{
// public:
// 	action_t action;
// 	MCTNode* parent;
// 	vector<MCTNode*> children;
// 	uint16_t wins, visits;

// 	MCTNode(){
// 		parent = nullptr;
// 		wins = 0;
// 		visits = 0;
// 	};

// 	MCTNode(action_t actionPlayed, MCTNode* parentNode){
// 		action = actionPlayed;
// 		parent = parentNode;
// 		wins = 0;
// 		visits = 0;
// 	}

// 	~MCTNode(){}

// 	void update(int result){
// 		visits++;
// 		if (result)
// 			wins++;
// 	}

// 	bool is_leaf(){
// 		return children.size() == 0;
// 	}

// 	bool has_parent(){
// 		return parent != nullptr;
// 	}

// 	MCTNode& operator=(const MCTNode& node){
// 		action = node.action;
// 		parent = node.parent;
// 		children = node.children;
// 		wins = node.wins;
// 		visits = node.visits;
// 		return *this;
// 	}	

// private:
// };

// template<class state_t, class action_t>
// class Dragon{
// public:
// 	uint16_t maxIter;

// 	Dragon(){
// 		maxIter = MAX_ITERATIONS;
// 	}

// 	void search(state_t currState){
// 		MCTNode root(action_t("none"), nullptr);
// 		MCTNode node();
// 		state_t state;
// 		for (uint16_t iter = 1; iter <= maxIter; ++iter){

// 			node = root;
// 			state = currState;

// 			while (!node.is_leaf()){
// 				node = select_policy(node, depth);
// 				state.update(node.action);
// 				depth++;
// 			}

// 		}
// 	}

// 	MCTNode select_policy(node, depth){
// 		return MCTNode(action_t(""), nullptr);
// 	}
// private:
// };

#endif
