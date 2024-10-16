mkdir -p automata-2-sym
rm -f automata-2-sym/*.txt

for I in `seq 1 20`; do
	for STATES_NUM in 2 4 8 10; do
		python3 ./rand_automaton.py\
			--alphabet a,b\
			--states-num $STATES_NUM\
			--max-weight 10\
			--min-weight -10\
			--edges-num $((($STATES_NUM * 2) + ($RANDOM % (2*$STATES_NUM)) + 1))\
			> automata-2-sym/A${I}-${STATES_NUM}.txt;
	done;
done

# mkdir -p automata-4-sym
# rm -f automata-4-sym/*.txt
# 
# 
# for I in `seq 1 20`; do
# 	for STATES_NUM in 2 4 8 10; do
# 		python3 ./rand_automaton.py\
# 			--alphabet a,b,c,d\
# 			--states-num $STATES_NUM\
# 			--max-weight 10\
# 			--min-weight -10\
# 			--edges-num $((($STATES_NUM * 4) + ($RANDOM % (2*$STATES_NUM)) + 1))\
# 			> automata-4-sym/A${I}-${STATES_NUM}.txt;
# 	done;
# done
# 
# mkdir -p automata-large-4-sym
# rm -f automata-large-4-sym/*.txt
# for I in `seq 1 500`; do
# 	for STATES_NUM in 250 500 750 1000; do
# 		python3 ./rand_automaton.py\
# 			--alphabet a,b,c,d\
# 			--states-num $STATES_NUM\
# 			--max-weight 10\
# 			--min-weight -10\
# 			--edges-num $((($STATES_NUM * 4) + ($RANDOM % (2*$STATES_NUM)) + 1))\
# 			> automata-large-4-sym/A${I}-${STATES_NUM}.txt;
# 	done;
# done
# 
# for I in `seq 501 1000`; do
# 	for STATES_NUM in 250 500 750 1000; do
# 		python3 ./rand_automaton.py\
# 			--alphabet a,b,c,d\
# 			--states-num $STATES_NUM\
# 			--max-weight 10\
# 			--min-weight -10\
# 			--edges-num $((($STATES_NUM * 4) + ($RANDOM % (2*$STATES_NUM)) + 1))\
# 			--constant\
# 			> automata-large-4-sym/A${I}-${STATES_NUM}.txt;
# 	done;
# done
# 
# mkdir -p automata-large-16-sym
# rm -f automata-large-16-sym/*.txt
# for I in `seq 1 500`; do
# for STATES_NUM in 250 500 750 1000; do
# 	python3 ./rand_automaton.py\
# 		--alphabet a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16\
# 		--states-num $STATES_NUM\
# 		--max-weight 10\
# 		--min-weight -10\
# 		--edges-num $((($STATES_NUM * 16) + ($RANDOM % (2*$STATES_NUM)) + 1))\
# 		> automata-large-16-sym/A${I}-${STATES_NUM}.txt;
# done;
# done
# 
# for I in `seq 501 1000`; do
# for STATES_NUM in 250 500 750 1000; do
# 	python3 ./rand_automaton.py\
# 		--alphabet a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16\
# 		--states-num $STATES_NUM\
# 		--max-weight 10\
# 		--min-weight -10\
# 		--edges-num $((($STATES_NUM * 16) + ($RANDOM % (2*$STATES_NUM)) + 1))\
# 		--constant\
# 		> automata-large-16-sym/A${I}-${STATES_NUM}.txt;
# done;
# done
