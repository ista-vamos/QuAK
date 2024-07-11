/*
template <typename T_key, typename T_value>
std::string MapStd<T_key, T_value>::toString (std::string (*f_key) (T_key key), std::string (*f_value) (T_value value)) const {
	std::string s = "";
	for (std::pair<T_key, T_value> pair : this->all){
		s.append("\n\t\t");
		s.append(f_value(pair.second));
		s.append(" -> ");
		s.append(f_key(pair.first));
	}
	return s;
}
*/

