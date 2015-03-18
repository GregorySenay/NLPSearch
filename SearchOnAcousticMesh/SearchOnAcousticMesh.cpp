/*
    --------------------------------------------------------
    SearchOnAcousticMesh : Search word or phoneme sequence in mesh file (confusion network)
    --------------------------------------------------------

    Copyright (C) 2012 GREGORY SENAY

    ..................................................................

    SearchOnAcousticMesh is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
    ..................................................................

    For any publication related to scientific work using this software,
    the following reference paper must be mentioned in the bibliography: 
    
    ..................................................................
                              
    Contact :
              GREGORY SENAY - LIA - UNIVERSITE D'AVIGNON
              AGROPARC BP1228 84911  AVIGNON  CEDEX 09  FRANCE
              firstname.lastname@univ-avignon.fr
    ..................................................................
*/

#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <string.h>
#include <math.h>
using namespace std;

string MESH_HEADER_1 = "name";
string MESH_HEADER_2 = "numaligns";
string MESH_HEADER_3 = "posterior";
string MESH_HEADER_4 = "align";
float CUT_VALUE      = -1;
float DISTANCE_MAX   = 0;
unsigned short DISTANCE_MAX_VALUE = 0;
unsigned short NB_PHONEME_MIN_POUR_DISTANCE = 4;

bool mode_avec_pause = false;

string FILE_NAME 	 = "";
unsigned int NODE_NUMBER = 0;
float PHON_VALUE_MIN = 0.01;
bool DEBUG  = false;
bool DEBUG2 = false;

unsigned int nb_finded_on_node     = 0;
unsigned int nb_finded_on_node_all = 0;
unsigned int MAX_SEARCH_FOR_A_PHONETISATION  = 15000;
unsigned int MAX_SEARCH_FOR_A_WORD           = 150000000;

map <string,float> pho_N_value;
vector < map <string,float> > my_Mesh;

/************************/
struct word_phon{
	unsigned int number;
	vector <string> phonetisation;		
};

vector<std::string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim) {
    vector<std::string> elems;
    return split(s, delim, elems);
}

/**************************/

//map <string,int> map_dic; // ecriture du mot et son numero 
map <string, struct word_phon> map_phon; // mot <=> et sa struc de phonetisation

void loadPhon(string phonFileName,bool numero_de_mot){
	ifstream phonFile (phonFileName.c_str());
	if (!phonFile.is_open()){
		string error = "Error opening file :";
		error += phonFileName;
		perror (error.c_str());
	}
	else
	{
		while (phonFile.good()) // lecture ligne a ligne
		{
				string tmp;
				getline(phonFile,tmp);
				/*DECOUPAGE DE LA LIGNE DE PHONETISATION*/
				int first_tab  = tmp.find_first_of("\t"); // position du permier tab
				int second_tab = tmp.rfind("\t"); // ou est le second tab
				if(DEBUG)cout << tmp<<endl;
				string num;
				string word;
				string phon;
				if(!numero_de_mot){
					word = tmp.substr(0,first_tab); // le num du mot
					phon = tmp.substr(first_tab+1,second_tab-(first_tab+1)); // le mot 
//					phon = tmp.substr(second_tab+1,(tmp.length()-1)-second_tab); // la phon du mot
				}else{
					num  = tmp.substr(0,first_tab); // le num du mot
					word = tmp.substr(first_tab+1,second_tab-(first_tab+1)); // le mot 
					phon = tmp.substr(second_tab+1,(tmp.length()-1)-second_tab); // la phon du mot	
				}
				/*AJOUT DANS DICO du NOMBRE et du MOT*/

				/*AJOUT DANS LA STRUCTURE DES DIFFERENTES PHONETISATIONS*/
				map<string ,struct word_phon>::iterator it = map_phon.find(word); //cherche ce nombre
				if(it == map_phon.end()){ //si y est pas
					if(DEBUG)cout<<"vide: "<<word<<endl;
					struct word_phon wp;
					wp.phonetisation.push_back(phon);
					it = map_phon.insert(it,pair<string,struct word_phon>(word,wp));
				}else{
					if(DEBUG)cout<<"existe: "<< word<<endl;
					(*it).second.phonetisation.push_back(phon);
				}
			}
			phonFile.close();
		}
}
/*******************/
void loadMesh(string mesh_File){
	ifstream mesh (mesh_File.c_str());
	bool pause_en_fin = false;
	if (!mesh.is_open()){
		string error = "Error opening file :";
		error += mesh_File.c_str();
		perror (error.c_str());
	}
	else
	{
		while (!mesh.eof()) // lecture ligne a ligne
		{
			string tmp;
			getline(mesh,tmp);
			vector<std::string> tab = split(tmp,' ');
			if( tab.size() ){
//				cout << tab[0]<< " " << tab[1] << endl;
				if( tab[0].compare(MESH_HEADER_1) == 0 )
					FILE_NAME = tab[1];
				if( tab[0].compare(MESH_HEADER_2) == 0 )
					NODE_NUMBER = atoi(tab[1].c_str());
				//if( tab[0].compare(MESH_HEADER_3) == 0 )
				//	; // RAS
				if( tab[0].compare(MESH_HEADER_4) == 0 ){
					map <string,float> new_pho_N_value;
					for(unsigned int p=2;p<tab.size();p+=2){
//						cout<< tab[p]<< " " << atof(tab[p+1].c_str()) << " ";
						float phon_value = atof(tab[p+1].c_str());
						if(phon_value >= PHON_VALUE_MIN ){
							pair <string,float> new_pho (tab[p],phon_value);
							new_pho_N_value.insert(new_pho);
						}
					}
					my_Mesh.push_back(new_pho_N_value);
//					cout << endl;
				}
			}
		}
	}
}

void DisplayMesh(){
	for(unsigned short node=0;node<my_Mesh.size();node++){
		// my_Mesh[node]  ==> la map
		map<string,float>::iterator it;
		cout << node ;
		for(it=my_Mesh[node].begin();it != my_Mesh[node].end();it++){
			cout<< " " << (*it).first << " " << (*it).second ;
		}
		cout <<endl;
	}
}

int word_finded=0;


/*
map <string,float> pho_N_value;
vector < map <string,float> > my_Mesh; // les nodes /qui ont des < pho et values >
map <string, struct word_phon> map_phon; 
*/
unsigned int nb_pau                 = 0;
unsigned int distance_actuelle      = 0;
unsigned int save_indice_node_end   = 0;
unsigned int save_indice_node_begin = 0;
unsigned int indice_node_begin_actu = 0;

struct finded_word{
	//string word;
	unsigned short nb_phoneme;
	unsigned short nb_phoneme_trouve;
	unsigned short nb_pause;
	float value;
	unsigned short begin;
	unsigned short end;
};

vector <finded_word> phonetisation_touve;

float search_on_suite( vector<string> phonetisations , unsigned int indice_phoneme,unsigned int indice_node, float value, unsigned int a_trouver){
	if(indice_phoneme == a_trouver && value > 0){
		if(DEBUG2)
		cout<<"OK TROUVE "<<a_trouver<<" " <<" "<<nb_pau<<" "<< value<<" "<<(value/(a_trouver+nb_pau))<<endl;
		nb_finded_on_node++;
		nb_finded_on_node_all++;
		struct finded_word new_phon_finded;
		//new_phon_finded.word              = ;
		new_phon_finded.nb_phoneme        = a_trouver;
		new_phon_finded.nb_phoneme_trouve = indice_phoneme-distance_actuelle;
		new_phon_finded.nb_pause          = nb_pau;
		if(mode_avec_pause)
			new_phon_finded.value             = value/(a_trouver+nb_pau); // avec prise en compte des pauses
		else
			new_phon_finded.value             = value/a_trouver; 		// sans prise en compte des pauses
		//new_phon_finded.value             = value;
		new_phon_finded.begin             = save_indice_node_begin;
		new_phon_finded.end               = indice_node-1;

		phonetisation_touve.push_back(new_phon_finded);
		return value/(a_trouver+nb_pau);
	}
	if( nb_finded_on_node_all > MAX_SEARCH_FOR_A_WORD){
		if(DEBUG2)cerr << "MAXIMUM ATTEIND 1" << endl;
		return 0;
	}	
	if(nb_finded_on_node > MAX_SEARCH_FOR_A_PHONETISATION){
		if(DEBUG2)cerr << "MAXIMUM ATTEIND 2" << endl;
		return 0;
	}
	if( indice_node == my_Mesh.size()){	// Si on arrive au bout ! c'es tqu'on a rien trouve
		if(DEBUG2)cout<< "BREAK END\n"<<endl;
		return 0;
	}
	if(nb_pau > phonetisations.size()){ // Si on trouve trop de pause ... c'est pas bon ;-)
		if(DEBUG2)cout<< "BREAK PAUSE\n"<<endl;	// ATTENTTION diviser par 3 au depart
		return 0;
	}
	float max = 0;
	float tmp = 0;
	map <string,float> :: iterator it = my_Mesh[indice_node].find(phonetisations[indice_phoneme]);
	//cout<<"W "<< indice_node<<" "<<indice_phoneme<<" "<< a_trouver <<endl;
	if( it != my_Mesh[indice_node].end() ){ // SI JE TROUVE LE PHONEME DANS CE NOEUD;
		if(DEBUG2){
				for(unsigned int i=0;i<indice_phoneme+nb_pau;i++)cout <<"\t";
			cout<< "Trouve " << phonetisations[indice_phoneme] <<" "<<indice_phoneme << " : " << indice_node << " " <<(*it).second<<" " << (value+(*it).second)<< endl;
		}	
		tmp = search_on_suite(phonetisations,indice_phoneme+1,indice_node+1,value+(*it).second,a_trouver);
	}
	if(tmp > max){ max=tmp;	}
	it       = my_Mesh[indice_node].find("*DELETE*");
	if(indice_node > 0 && indice_phoneme >0){
		if( it != my_Mesh[indice_node].end() ){ // JE TROUVE LE PHONEME DANS CE NOEUD;
			if(DEBUG2){
				for(unsigned int i=0;i<indice_phoneme+nb_pau;i++)cout <<"\t";
				if(DEBUG2)cout<< "Trouve PAUSE : " << indice_phoneme <<" " << indice_node << " "<<  (*it).second <<" "<< (value+(*it).second)<< endl;
			}
			nb_pau ++;
			if(mode_avec_pause)
				tmp = search_on_suite(phonetisations,indice_phoneme,indice_node+1,value+(*it).second,a_trouver); // Score avec prise en compte du score de la pause
			else
				tmp = search_on_suite(phonetisations,indice_phoneme,indice_node+1,value              ,a_trouver); // Score sans prise en compte du score de la pause
			nb_pau --;
		}
	}
	if(tmp > max){ max=tmp;	}
	/*Si je ne trouve pas alors je me fis a la distance d edition */
	if(distance_actuelle < DISTANCE_MAX){
		distance_actuelle++;
		tmp = search_on_suite(phonetisations,indice_phoneme+1,indice_node+1,value,a_trouver);
		distance_actuelle--;
	}	
	if(tmp > max){ max=tmp;	}
	
	return max;	
}

/*************************/
float search_on_node(vector<string> phonetisation){
	float max = 0;
	if(DISTANCE_MAX_VALUE > 0){
		DISTANCE_MAX = floor( 0.5 + ((float)phonetisation.size()/DISTANCE_MAX_VALUE));
		//DISTANCE_MAX = 1;
	}else{	
		DISTANCE_MAX = 0;
	}
	if(phonetisation.size() < NB_PHONEME_MIN_POUR_DISTANCE){  // Si le nomnre de phonemes est trop petit on cherche la chaine extact
		 DISTANCE_MAX = 0;
	}
	for(unsigned int n=0;n<my_Mesh.size();n++){
		nb_pau=0;
		nb_finded_on_node = 0;
		save_indice_node_begin = n;
		float tmp = search_on_suite( phonetisation,0,n,0,phonetisation.size() );
		if(tmp>max){
			max=tmp;
			save_indice_node_begin = n;
		}
	}
	return max;
}



float search(string word){
	word_finded=0;
	nb_finded_on_node_all=0;

	phonetisation_touve.clear();
	map <string,struct word_phon>:: iterator it;
	it = map_phon.find(word);
	float max = 0;
	unsigned int max_nb_phoneme = 0;
	if(it != map_phon.end())
	for(unsigned int p=0;p<(*it).second.phonetisation.size();p++){
		string phonetisation = (*it).second.phonetisation[p];
		vector <string> v_phonetisation = split(phonetisation,' ');
		 //cout << "\t"<< phonetisation <<endl;
//		searchPhonetisation(v_phonetisation);
		float tmp = 0;
		tmp = search_on_node(v_phonetisation);
		//cout << tmp << endl;
		if(tmp > max){
			word_finded++;
			max=tmp;
			max_nb_phoneme = v_phonetisation.size();
		}
		if( nb_finded_on_node_all > MAX_SEARCH_FOR_A_WORD){
			return max;
		}
	}
	//if( word_finded > 0){
	//	if( max > CUT_VALUE)
	//	cout << word << " : " << max <<" : "<< word_finded << " : "<< max_nb_phoneme << " : " << save_indice_node_begin << " -> " << save_indice_node_end <<endl;
	//}
	return max;
}

/***************************/

void affiche_one(struct finded_word phonetisation,string word){
	cout<<word<<" ";
	cout<< phonetisation.value     		<<" (";
	cout<< phonetisation.nb_phoneme		<<";";
	cout<< phonetisation.nb_phoneme_trouve	<<";";
	cout<< phonetisation.nb_pause		<<";";
	cout<< phonetisation.begin		<<"->";
	cout<< phonetisation.end		<<";";
	cout<< phonetisation_touve.size()	<<")"<<endl;
}

void affiche_phonetisation(vector <struct finded_word> phonetisation_touve,string word){

	//cout <<"____"<<endl;
	vector <struct finded_word> best;
	struct finded_word best_on_borne;
	/*Ajour d un meilleur par default*/
	for(unsigned int i=0;i<phonetisation_touve.size();i++){
		bool a_ajouter = true; // Pour l instant j'ai pas trouve meilleur
		for(unsigned int b=0;b<best.size() && a_ajouter==true;b++){ // Je cherche dans la liste des meilleurs
			if( best[b].begin == phonetisation_touve[i].begin &&
					phonetisation_touve[i].end == best[b].end &&
						phonetisation_touve[i].value >= best[b].value){
				a_ajouter = false;
				best[b] = phonetisation_touve[i];
			}
		}
		if(a_ajouter==true){
			best.push_back(phonetisation_touve[i]);
		}
	}

	for(unsigned int b1=0;b1<best.size() ; b1++){
		bool meilleur = false;
		for(unsigned int b2=0 ; b2<best.size() ; b2++){
			if(best[b2].begin <= best[b1].begin && best[b1].begin < best[b2].end ){ // Si y a chevauchemen
				if( b1 != b2 && best[b2].value > best[b1].value ){
						meilleur=true;
				}
				if( b1 != b2 && best[b2].value == best[b1].value && best[b1].nb_pause < best[b2].nb_pause){
						meilleur=true;
				}
			}
			if(best[b2].begin < best[b1].end   && best[b1].end   <= best[b2].end){
				if( b1 != b2 && best[b2].value > best[b1].value ){
						meilleur=true;
				}
				if( b1 != b2 && best[b2].value == best[b1].value && best[b1].nb_pause < best[b2].nb_pause){
						meilleur=true;
				}
			}
			if(best[b1].begin <= best[b2].begin && best[b2].begin < best[b1].end ){ // Si y a chevauchemen
				if( b1 != b2 && best[b2].value > best[b1].value ){
						meilleur=true;
				}
				if( b1 != b2 && best[b2].value == best[b1].value && best[b1].nb_pause < best[b2].nb_pause){
						meilleur=true;
				}
			}
			if(best[b1].begin < best[b2].end   && best[b2].end   <= best[b1].end){
				if( b1 != b2 && best[b2].value > best[b1].value ){
						meilleur=true;
				}
				if( b1 != b2 && best[b2].value == best[b1].value && best[b1].nb_pause < best[b2].nb_pause){
						meilleur=true;
				}
			}
		}
		if(meilleur == false){
			if( best[b1].value >= CUT_VALUE)
				affiche_one(best[b1],word);
		}
	}
}

void find_words(string words_list){
	ifstream wordsFile (words_list.c_str());
	if (! wordsFile.is_open() ){
		string error = "Error opening file en liste ";
		error += words_list;
		perror (error.c_str());
	}
	else
	{
		while (wordsFile.good())
		{
			string atrouver;
			getline(wordsFile,atrouver);
			if(atrouver.compare("")!=0){
				cerr<<"------"<<atrouver<<"------"<<endl;
				phonetisation_touve.clear();
				float t = search(atrouver);
				//cout<<"Total :" << t <<endl;
				//if(t > 0.001)
				//	cout<<atrouver<<" - "<<t<<" "<< t <<endl;
				affiche_phonetisation(phonetisation_touve,atrouver);
			}
		}
		cout << "----END----" << endl;
	}
	wordsFile.close();
}

void help(char * a){
	cout<< a << " Usage:"<<endl;
//	cout<< " <i:acoustic_mesh - .phol> <i:phonetisations> <i:liste_a_rechercher>"<<endl;
	cout<< "\t--mesh  " << "Reseau de confusion de phonemes" <<endl;
	cout<< "\t--phon  " << "Mot et phonetisations" <<endl;
	cout<< "\t--liste " << "liste des mots a rechercher" <<endl;
	cout<< "\t--seuil " << " ::optionelle:: float [0;1]" <<endl;
	cout<< "\t--de "<< " distance d edition ::optionelle:: int [0;inf]" <<endl;
	cout<< "\tVERSION 2.3 *DELETE* - Ajout de la distance d edition"<<endl;
	cout<< "\t            - score sans score de la pause"<<endl;
	cout<< "\t            - limite dans la recherche d une phonetisation a "<< MAX_SEARCH_FOR_A_PHONETISATION <<endl;
	cout<< "\t            - limite dans la recherche d un mot "<< MAX_SEARCH_FOR_A_WORD <<endl;
	cout<< "\t            - lecture des phonemes superieurs a "<< PHON_VALUE_MIN <<endl;
}
/***************************/

int main(int argc, char ** argv){
	
	string o_mesh  = "--mesh";
	string o_phon  = "--phon";
	string o_liste = "--liste";
	string o_value = "--seuil";
	string o_de    = "--de";
	
	string mesh;
	string phon;
	string liste;
	short arrg = 0;
	short de   = 0;
	for(int a=0; a<argc; a++){
		if(strcmp(argv[a],"-h")==0||strcmp(argv[a],"--h")==0||strcmp(argv[a],"--help")==0||strcmp(argv[a],"-help")==0){
			help(argv[0]);exit(0);
		}
		if( o_mesh.compare(argv[a]) == 0){
			if(a+1 >= argc){help(argv[0]);exit(0);}
			mesh = argv[a+1];arrg++;
		}
		if( o_phon.compare(argv[a]) == 0){
			if(a+1 >= argc){help(argv[0]);exit(0);}
			phon = argv[a+1];arrg++;
		}
		if(o_liste.compare(argv[a]) == 0){
			if(a+1 >= argc){help(argv[0]);exit(0);}
			liste = argv[a+1];arrg++;
		}
		if( o_value.compare(argv[a]) == 0){
			if(a+1 >= argc){help(argv[0]);exit(0);}
			//value = argv[a+1];
			CUT_VALUE = atof(argv[a+1]);
		}
		if( o_de.compare(argv[a]) == 0){
			if(a+1 >= argc){help(argv[0]);exit(0);}
			de = atoi(argv[a+1]);
			if( de < 1) de = 0;
			DISTANCE_MAX_VALUE = de;
		}
	}
	if( arrg > 2){
		loadMesh(mesh);
		loadPhon(phon,false);
		find_words(liste);
	}else{
		help(argv[0]);exit(0);
	}
//	DisplayMesh();
	return 0;
}
