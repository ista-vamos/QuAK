mkdir -p automata-2-sym
rm automata-2-sym/*.txt

for I in `seq 1 20`; do
	for STATES_NUM in 2 4 8 10; do
		python ./rand_automaton.py\
			--alphabet a,b\
			--states-num $STATES_NUM\
			--max-weight 10\
			--min-weight -10\
			--edges-num $(($STATES_NUM * 2 + $RANDOM % 2*$STATES_NUM))\
			> automata-2-sym/A${I}-${STATES_NUM}.txt;
	done;
done

mkdir -p automata-4-sym
rm automata-4-sym/*.txt


for I in `seq 1 20`; do
	for STATES_NUM in 2 4 8 10; do
		python ./rand_automaton.py\
			--alphabet a,b,c,d\
			--states-num $STATES_NUM\
			--max-weight 10\
			--min-weight -10\
			--edges-num $(($STATES_NUM * 4 + $RANDOM % 2*$STATES_NUM))\
			> automata-4-sym/A${I}-${STATES_NUM}.txt;
	done;
done

mkdir -p automata-large-4-sym
rm automata-large-4-sym/*.txt
for I in `seq 1 20`; do
	for STATES_NUM in 1000 5000; do
		python ./rand_automaton.py\
			--alphabet a,b,c,d\
			--states-num $STATES_NUM\
			--max-weight 10\
			--min-weight -10\
			--edges-num $(($STATES_NUM * 2 + $RANDOM % 2*$STATES_NUM))\
			> automata-large-4-sym/A${I}-${STATES_NUM}.txt;
	done;
done

