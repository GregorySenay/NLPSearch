/*
    --------------------------------------------------------
    HTK_Search : Search word or phoneme sequence in lattice (HTK)
    --------------------------------------------------------

    Copyright (C) 2013 GREGORY SENAY

    ..................................................................

    HTK_Search is free software; you can redistribute it and/or modify
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
                              
    Contact :
              GREGORY SENAY - LIA - UNIVERSITE D'AVIGNON
              AGROPARC BP1228 84911  AVIGNON  CEDEX 09  FRANCE
              gregory.senay@univ-avignon.fr
    ..................................................................
*/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <sstream>
#include<fstream>

using namespace std;


bool DEBUG = false;
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
/************************/
map <string, struct word_phon> map_phon; // mot <=> et sa struc de phonetisation

void loadPhon(string phonFileName,bool numero_de_mot){
	ifstream phonFile (phonFileName.c_str());
	if (!phonFile.is_open())
		perror ("Error opening file");
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

/*****************************************/


struct HTK_link{
	string pho; //ou mot
	float value;
	unsigned int num;
	unsigned int begin;
	unsigned int end;
	bool this_is_the_end;
	//vector <struct phoneme *> nexts;
	map< string,struct HTK_link* > nexts_map;
};

unsigned int NB_LINK;

map <unsigned int,float> node_time;
map <unsigned int,vector<unsigned int> > node_begin_nodes_number;

struct HTK_link ** all_link;

unsigned int finded_word = 0;
/************************/

void affichePremierInHTK(struct HTK_link * node ){
	if(node->this_is_the_end == false){ // tant que je ne suis pas dans un noeud final
		cout<< node->pho << " " ;
		if(node->nexts_map.size() > 0){
			map<string,struct HTK_link* >::iterator it = node->nexts_map.begin();
			for(; it != node->nexts_map.end() ; it++)
				affichePremierInHTK((struct HTK_link*)(*it).second);
		}else{
			cerr<< "NOEUD VIDE AIE " << node->num<< endl;
		}
	}
}

void parcoursHTK(){
	for(unsigned int n=0;n<NB_LINK;n++){
		if(	all_link[n]->begin == 0){
			cout << "depart en " << n << endl;
			affichePremierInHTK(all_link[n]);
			cout<<endl;
		}
	}
}
///tt rr ou vv eu dd an ll eu mm aa nn
int search_on_all_node(vector <string> v_phonetisation,int nb_phoneme,struct HTK_link* node){
		if( nb_phoneme == v_phonetisation.size()-1){
	
				map<string,struct HTK_link* >::iterator it = node->nexts_map.begin();
				
				if(DEBUG)cout << node->pho         << " " ;
//				<< node->nexts_map.size() << " "
//				<< (*it).second->pho      << " "
//				<< v_phonetisation[nb_phoneme] <<endl;
				
				it = node->nexts_map.find(v_phonetisation[nb_phoneme]);
				if(it != node->nexts_map.end()){ //si y est
					if(DEBUG)cout << v_phonetisation[nb_phoneme] << endl;
					if(DEBUG)cout << "OK" << endl;
					finded_word++;
					return 1;
				}
				if(DEBUG)cout << " X" << v_phonetisation.size();
		}	
		
		if( nb_phoneme == v_phonetisation.size()){
			if(DEBUG)cout << node->pho << " OK " << endl;
			finded_word++;
			return 1;
		}
		if(node->this_is_the_end == true){
			return 0;
		}

		map<string,struct HTK_link* >::iterator it =
								node->nexts_map.find(v_phonetisation[nb_phoneme]);
		if(it == node->nexts_map.end()){ //si y est pas
			;
		}else{
			if(DEBUG)cout << node->pho << " - ";
			search_on_all_node(v_phonetisation,nb_phoneme+1,(*it).second);
			if(DEBUG)cout<<endl;
		}
		/*SI c est un noeud vide -pau-$*/
		
		it = node->nexts_map.find("-pau-");
		if(it == node->nexts_map.end()){ //si y est pas
				;
		}else{
			if(DEBUG)cout <<	node->pho << " ";
			search_on_all_node(v_phonetisation,nb_phoneme,(*it).second);
			if(DEBUG)cout<<endl;
		}
	return 0;
}

int search_on_all_node(vector <string> v_phonetisation){
	map<string,struct HTK_link* >::iterator it;
	for(unsigned int n=0;n<NB_LINK;n++){
		if(all_link[n]->this_is_the_end != true && all_link[n]->pho == v_phonetisation[0]){
		/*si je suis le bon phoneme */
				it = all_link[n]->nexts_map.find(v_phonetisation[1]);
				if(it == all_link[n]->nexts_map.end()){ //si y est pas
						;
				}else{
					if(DEBUG)cout << n << ": " <<	all_link[n]->pho<< " " ;
					search_on_all_node(v_phonetisation,2,(*it).second);
					if(DEBUG)cout<<endl;
				}
				/*SI c est celui d'apres est un noeud vide -pau- */
				it = all_link[n]->nexts_map.find("-pau-");
				if(it == all_link[n]->nexts_map.end()){ //si y est pas
						
				}else{
					if(DEBUG)cout << n << ": " << all_link[n]->pho << " " ;
					search_on_all_node(v_phonetisation,1,(*it).second);
					if(DEBUG)cout<<endl;
				}
		}
	}
	return 0;
}


float search(string word){
	short word_finded=0;
	map <string,struct word_phon>:: iterator it;
	it = map_phon.find(word);
	float max = 0;
	finded_word = 0;
	for(unsigned int p=0;p<(*it).second.phonetisation.size();p++){
		string phonetisation = (*it).second.phonetisation[p];
		vector <string> v_phonetisation = split(phonetisation,' ');
		// cout << "\t"<< phonetisation <<" " ;
//		searchPhonetisation(v_phonetisation);
		float tmp = 0;
		
		tmp = search_on_all_node(v_phonetisation);
//		cout << tmp << endl;
		if(tmp > max){ max=tmp;}
	}
	if( finded_word > 0){
//		if( max > CUT_VALUE)
		cout << word << " : "<< finded_word <<endl;
	}
	return max;
}

/***********************************/
void linkHTK(){
//	cout << NB_LINK <<endl;
	all_link[1]->pho = "-pau-";
	for(unsigned int n=0;n<NB_LINK;n++){
		//	cout << "Node num: "<< all_link[n]->num <<endl;
		//	cout << "Node sui: "<< all_link[n]->end <<endl;
//			cout << "\t J= : " << n <<" "<< all_link[n]->end <<endl;
			map<unsigned int,vector<unsigned int> >::iterator it =  node_begin_nodes_number.find(all_link[n]->end);
		//	cout << "Vector : "<< (*it).second.size()<<endl;
			if(all_link[n]->end == 1){
				all_link[n]->this_is_the_end=true;
			}else
				for(unsigned int suiv=0;suiv<(*it).second.size();suiv++){
					//On ajoute dans le pointeur nexts_maps les pointeurs sur les suivants
					// Avec comme id le nom/pho
	//				cout << "\t E= " << (*it).second[suiv] <<endl;
					string pho = all_link[(*it).second[suiv]]->pho;
	//				cout << "\t" << (*it).second[suiv] <<" " << pho <<endl;
					//struct HTK_link* new_link = all_link[(*it).second[suiv]];
					//cout<< all_link[n]->nexts_map.size() << endl;
					all_link[n]->nexts_map.insert(pair<string,struct HTK_link* >(pho,all_link[(*it).second[suiv]]));
			}
	}
}
/***********************************/
void loadHTK(string HTK_File){
	if(DEBUG)cout<<HTK_File<<endl;
	ifstream htk (HTK_File.c_str());
	bool pause_en_fin = false;
	if (!htk.is_open()){
		string error = "Error opening file :";
		error += HTK_File.c_str();
		perror (error.c_str());
	}
	else
	{
		while (!htk.eof()) // lecture ligne a ligne
		{
			string tmp;
			getline(htk,tmp);
			vector<std::string> tab = split(tmp,'\t');
			vector<std::string> info = split(tmp,'=');
			if(info.size() == 2){
				//cout<<"HEADERS: "<<tmp<<endl;
			}else
			if( tab.size() ){
				//cout<<tmp<<endl;
				if( info[0].compare("N") == 0 ){
					vector<std::string> info1 = split(tab[0],'=');
					vector<std::string> info2 = split(tab[1],'=');
					unsigned int nb_node = atoi(info1[1].c_str());
					unsigned int nb_link = atoi(info2[1].c_str());
					all_link = new struct HTK_link*[nb_link];
					NB_LINK = nb_link;
				}
				if( info[0].compare("I") == 0 ){
					vector<std::string> info1 = split(tab[0],'=');
					vector<std::string> info2 = split(tab[1],'=');
					unsigned int num = atoi(info1[1].c_str());
					float temps      = atof(info2[1].c_str());
				//	cout << num << " <-> " << temps << endl;
					node_time.insert(pair<unsigned int,float>(num,temps));
				}
				if( info[0].compare("J") == 0 ){
					//cout << tmp << endl;
					vector<std::string> num_links  = split(tab[0],'=');
					vector<std::string> num_begins = split(tab[1],'=');
					vector<std::string> num_ends   = split(tab[2],'=');
					vector<std::string> words      = split(tab[3],'=');
					unsigned int num_link 	= atoi(num_links[1].c_str());
					unsigned int num_begin  = atoi(num_begins[1].c_str());
					unsigned int num_end	= atoi(num_ends[1].c_str());
					string word				= words[1];
					all_link[num_link] = new struct HTK_link;
					all_link[num_link]->pho   = word;
					all_link[num_link]->num   = num_link;
					all_link[num_link]->value = 0;
					all_link[num_link]->begin = num_begin;
					all_link[num_link]->end   = num_end;
					all_link[num_link]->this_is_the_end = false;
					node_begin_nodes_number;
					
					//en num_link on debute en num_begin
					map<unsigned int,vector<unsigned int> >::iterator it = node_begin_nodes_number.find(num_begin); //cherche ce nombre
				if(it == node_begin_nodes_number.end()){ //si y est pas
					if(DEBUG)cout<<"vide: "<<num_begin<<endl;
					vector<unsigned int> new_vector;
					new_vector.push_back(num_link);
					it = node_begin_nodes_number.insert(it,pair<unsigned int,vector<unsigned int> >(num_begin,new_vector));
					
				}else{
					if(DEBUG)cout<<"existe: "<< num_begin<<endl;
					(*it).second.push_back(num_link);
				}
					//cout<< num_link <<" "<< num_begin <<" " << num_end <<" " << word<<endl;
				}
			}
		}
	}
}
/***********************************/
void find_words(string words_list){
	ifstream wordsFile (words_list.c_str());
	if (! wordsFile.is_open() )
		perror ("Error opening file en liste");
	else
	{
		while (wordsFile.good())
		{
			string atrouver;
			getline(wordsFile,atrouver);
			if(atrouver!=""){
				float t = search(atrouver);
				cerr<<"------"<<atrouver<<"------"<<endl;
				//cout<<"------"<<atrouver<<"------"<<endl;
				//cout<<"Total :" << find(atrouver) <<endl;
				//if(t > 0.001)
				//	cout<<atrouver<<" - "<<t<<" "<< t <<endl;
			}
		}
	}
	wordsFile.close();
}
/***********************************/

void help(char * a){
	cout<< a << " Usage:"<<endl;
//	cout<< " <i:acoustic_mesh - .phol> <i:phonetisations> <i:liste_a_rechercher>"<<endl;
	cout<< "\t--read-htk  " << "Treillis formati htk phonemes" <<endl;
	cout<< "\t--phon  " << "Mot et phonetisations" <<endl;
	cout<< "\t--liste " << "liste des mots a rechercher" <<endl;
//	cout<< "\tVERSION 1 *DELETE* - Ajout de la distance d edition"<<endl;
}

int main(int argc, char ** argv){
	
	string o_htk  = "--read-htk";
	string o_phon  = "--phon";
	string o_liste = "--liste";
	
	string htk;
	string phon;
	string liste;
	short arrg = 0;
	short de   = 0;
	for(int a=0; a<argc; a++){
		if(strcmp(argv[a],"-h")==0||strcmp(argv[a],"--h")==0||strcmp(argv[a],"--help")==0||strcmp(argv[a],"-help")==0){
			help(argv[0]);exit(0);
		}
		if( o_htk.compare(argv[a]) == 0){
			if(a+1 >= argc){help(argv[0]);exit(0);}
			htk = argv[a+1];arrg++;
		}
		if( o_phon.compare(argv[a]) == 0){
			if(a+1 >= argc){help(argv[0]);exit(0);}
			phon = argv[a+1];arrg++;
		}
		if(o_liste.compare(argv[a]) == 0){
			if(a+1 >= argc){help(argv[0]);exit(0);}
			liste = argv[a+1];arrg++;
		}
	}
	if( arrg > 1){
		loadHTK(htk);
		linkHTK();
		//parcoursHTK();
		loadPhon(phon,false);
		find_words(liste);
	}else{
		help(argv[0]);exit(0);
	}
//	DisplayMesh();
	return 0;
}
