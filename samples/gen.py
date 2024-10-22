import random
import string
import os
from collections import deque

def find_reachable_states(transitions, initial_state, num_states):
    """Find all states reachable from the initial state using BFS."""
    reachable = set([initial_state])
    queue = deque([initial_state])
    
    while queue:
        current = queue.popleft()
        for _, _, source, target in transitions:
            if source == current and target not in reachable:
                reachable.add(target)
                queue.append(target)
                
    return reachable

def ensure_connectivity(transitions, num_states, alphabet, min_weight, max_weight):
    """Ensure all states are reachable from state 0 by adding necessary transitions."""
    initial_state = 0
    reachable = find_reachable_states(transitions, initial_state, num_states)
    
    # Keep adding transitions until all states are reachable
    while len(reachable) < num_states:
        # Find an unreachable state
        unreachable = set(range(num_states)) - reachable
        target = random.choice(list(unreachable))
        
        # Add a transition from a reachable state to the unreachable state
        source = random.choice(list(reachable))
        letter = random.choice(alphabet)
        weight = random.uniform(min_weight, max_weight)
        transitions.append((letter, weight, source, target))
        
        # Update reachable states
        reachable = find_reachable_states(transitions, initial_state, num_states)
    
    return transitions

def generate_complete_nondeterministic_automaton(num_states, alphabet_size, extra_transitions_per_state, min_weight, max_weight):
    states = list(range(num_states))
    alphabet = string.ascii_lowercase[:alphabet_size]
    transitions = []

    # Ensure each state has at least one transition for each letter (completeness)
    for state in states:
        for letter in alphabet:
            target = random.choice(states)
            weight = random.uniform(min_weight, max_weight)
            transitions.append((letter, weight, state, target))

    # Add extra transitions to create nondeterminism
    for state in states:
        for _ in range(extra_transitions_per_state):
            letter = random.choice(alphabet)
            target = random.choice(states)
            weight = random.uniform(min_weight, max_weight)
            transitions.append((letter, weight, state, target))

    # Ensure all states are reachable from the initial state
    transitions = ensure_connectivity(transitions, num_states, alphabet, min_weight, max_weight)

    return transitions

def save_automaton_to_file(transitions, filename):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    script_dir = os.path.join(script_dir, "rand")
    os.makedirs(script_dir, exist_ok=True)  # Create directory if it doesn't exist
    file_path = os.path.join(script_dir, filename)
    
    with open(file_path, 'w') as f:
        for letter, weight, source, target in transitions:
            f.write(f"{letter} : {weight:.4f}, {source} -> {target}\n")
    
    return file_path

def generate_automata_set(num_states, alphabet_size, num_automata, extra_transitions_per_state, min_weight, max_weight):
    for i in range(num_automata):
        transitions = generate_complete_nondeterministic_automaton(
            num_states, alphabet_size, extra_transitions_per_state, min_weight, max_weight
        )
        
        filename = f"{num_states}_{alphabet_size}_{i+1:04d}.txt"
        saved_path = save_automaton_to_file(transitions, filename)
        print(f"Automaton {num_states}_{alphabet_size} #{i+1:04d} saved to {saved_path}")

def main():
    num_automata_per_set = 1000
    min_weight, max_weight = -10.0, 10.0

    configurations = [
        (2, 2),  # (x, y) == x states, y letters
        (4, 2),
        (8, 2),
        (2, 4),
        (4, 4),
        (8, 4)
    ]

    for num_states, alphabet_size in configurations:
        print(f"\nGenerating {num_automata_per_set} automata with {num_states} states and {alphabet_size} letters:")
        extra_transitions_per_state = alphabet_size // 2
        generate_automata_set(num_states, alphabet_size, num_automata_per_set, extra_transitions_per_state, min_weight, max_weight)

if __name__ == "__main__":
    main()