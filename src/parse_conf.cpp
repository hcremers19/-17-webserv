#include "all_includes.hpp"

/* --------------------------------------------------------------------------------
Fonction principale du parsing du fichier de configuration
-------------------------------------------------------------------------------- */
void parsing(int argc, char **argv, Conf &data)
{
	parse_basic(argc, argv);
	data.read_file(argv[1]);
	data.init_file_pos();
	data.check_directive();
	data.stock_data();
	data.check_data();
}

/* --------------------------------------------------------------------------------
Vérifier si le fichier passé en argument du programme est valide
-------------------------------------------------------------------------------- */
void parse_basic(int argc, char **argv)
{
	if (argc != 2)
		throw ArgvErr();

	std::ifstream file(argv[1]);
	if (!file)
		throw ArgvErr();

	std::string name = std::string(argv[1]);
	if (name.find(".conf") == std::string::npos) // npos == -1 in size_t
		throw ArgvErr();
}

/* --------------------------------------------------------------------------------
Compter le nombre de mots dans la ligne "sentence" ("line" dans le .hpp)
-------------------------------------------------------------------------------- */
int count_words(std::string sentence)
{
	int ret = 0, i = -1;

	while (sentence[++i + 1])
		if (!isspace(sentence[i]) && isspace(sentence[i + 1]))
			ret++;
	if (!isspace(sentence[i]))
		ret++;
	return (ret);
}

/* --------------------------------------------------------------------------------
Vérifier si la string "word" est composée uniquement d'ints ou pas

/!\ Changer de fichier, n'est pas utilisée dans ce fichier ci
-------------------------------------------------------------------------------- */
bool my_atoi(std::string word)
{
	int i = -1;
	while (word[++i])
		if (!isdigit(word[i]))
			return false;
	return true;
}

/* --------------------------------------------------------------------------------
Retourner le premier mot de la ligne "line"
-------------------------------------------------------------------------------- */
std::string ft_first_word(std::string line)
{
	int i = 0, j;
	while (isspace(line[i]) && line[i++]);
	j = i - 1;
	while (!isspace(line[++j]) && line[j]);
	return (line.substr(i, j - i));
}

/* --------------------------------------------------------------------------------
Retourner le dernier mot de la ligne "line"

/!\ Changer de fichier, n'est pas utilisée dans ce fichier ci
-------------------------------------------------------------------------------- */
std::string ft_last_word(std::string line)
{
	int i = line.length() - 1, j;
	while (isspace(line[--i]) && line[i]);
	j = i + 1;
	while (--j > 0 && !isspace(line[j]));
	return (line.substr(j + 1, i - j));
}

/* --------------------------------------------------------------------------------
Return last part from line starting fron first word

/!\ À supprimer, n'est pas utilisée du tout
-------------------------------------------------------------------------------- */
std::string ft_last_part(std::string line)
{
	int pos = line.find(ft_first_word(line)), i = pos;

	while (line[++i]);
	return (line.substr(pos, i - pos));
}
