/*
    --------------------------------------------------------
    fusion_phon_N_treillis : Transform a word lattice in a phoneme lattice
    --------------------------------------------------------

    Copyright (C) 2012 GREGORY SENAY

    ..................................................................

    fusion_phon_N_treillis is free software; you can redistribute it and/or modify
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
#include <fstream>

using namespace std;


string file_out;
bool DEBUG 		= false;
bool DEBUG_TREILLIS	= false;
/** FONCTIONS BASIQUES **/
unsigned short temps_de_debut;
static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

/** STRUCTURES **/

typedef pair<unsigned short,unsigned short> doubletemps; // le param1 correspond au temps de debut du mot // td temps debut / tf temps de fin

typedef pair< pair<unsigned short,unsigned short>,unsigned short> doubletemps_N_num; // le param1 correspond au temps de debut du mot // td temps debut / tf temps de fin

struct Wordlist{
	string word;
	map <doubletemps,vector<vector<struct phoneme> > > phonetisations;  // un temps m tps de debut et de fin peut contenir plusieurs phonetisations ! donc vecteurs de vectors 
	map <doubletemps,vector<vector<struct phoneme> > > phonetisations_sans_pause;
};

struct phoneme{
	string pho;
	float score;
	unsigned short trame;
	unsigned short duree;
};

static bool compare_phoneme(struct phoneme a,struct phoneme b){
	if(a.pho.compare(b.pho)==0 && a.score == b.score && a.trame == b.trame && a.duree == b.duree)
		return true;
	return false;
}

static bool compare_vector_phonemes(vector<struct phoneme>va,vector<struct phoneme> vb){
	if(va.size() != vb.size())
		return false;
	for(unsigned int pho_num=0;pho_num < vb.size(); pho_num++){
	if(compare_phoneme(va[pho_num],vb[pho_num]) == false)
		return false;
	}
	return true;
}

static bool phonetisation_exist_dans_vector(vector<struct phoneme> a,vector <vector<struct phoneme> > vec_b){
//	cout << "COMPARATION " <<endl;
	
	for(unsigned int vec_num=0;vec_num < vec_b.size(); vec_num++){
		for(unsigned int p_a=0 ; p_a < vec_b[vec_num].size(); p_a++){
//			cout<<compare_phoneme(a[p_a],vec_b[vec_num][p_a])<<endl;
			if(compare_phoneme(a[p_a],vec_b[vec_num][p_a]) == false){
				//cout<<"False car "<< a[p_a].pho   << " " << vec_b[vec_num][p_a].pho <<endl;
				//cout<<"False car "<< a[p_a].score << " " << vec_b[vec_num][p_a].score <<endl;
				//cout<<"False car "<< a[p_a].trame << " " << vec_b[vec_num][p_a].trame <<endl;
				//cout<<"False car "<< a[p_a].duree << " " << vec_b[vec_num][p_a].duree <<endl;
				return true;
			}
		}
	}
	return false;
}

static void remove_doubles_phonetisation_dans_vector(vector <vector<struct phoneme> > & vec_b){
	for(unsigned int vec_num=0;vec_num < vec_b.size(); vec_num++){ 
		for(unsigned int vec_num2=vec_num+1;vec_num2 < vec_b.size(); vec_num2++){
			if(compare_vector_phonemes(vec_b[vec_num],vec_b[vec_num2])==true){
				vec_b.erase(vec_b.begin()+vec_num2);
				vec_num2--;
			}
		}
	}
}

map<string,struct Wordlist> Dico;

/** FONCTIONS SPECIFIQUES **/

static doubletemps_N_num cree_double_temps(unsigned short a,unsigned short b,unsigned short c){
	pair<unsigned short,unsigned short> p1(a,b);
	pair<pair<unsigned short,unsigned short>,unsigned short> all(p1,c) ;
	return all;
}

struct phoneme load_info_phoneme(string phon_name, string info2){
	
	vector<std::string> infos = split(info2,' ');
	string phoneme = infos[0];
	vector<std::string> value = split(infos[0],',');
	float score = atof(value[0].substr(1,value[0].length()).c_str());
	unsigned int debut = atoi(value[1].c_str());
	unsigned int duree = atoi(value[2].substr(0,value[2].length()-1).c_str());
	
	struct phoneme new_pho;
	new_pho.pho   = phon_name;
	new_pho.score = score; // ATTENTION SCORE CUMULE
	new_pho.trame = debut;
	new_pho.duree = duree;
	
	return new_pho;
}

static void normalisation_score_acoustique(vector<struct phoneme> & suite_phoneme_tmp){ // Pour enlever le score cumule
	for(unsigned short i=suite_phoneme_tmp.size()-1;i>0;i--)
		suite_phoneme_tmp[i].score-=suite_phoneme_tmp[i-1].score;
}

static void enleve_doublons(){
	//if(phonetisation_exist_dans_vector(new_phonetisation,(*it_temps).second) == false);

	map<string,struct Wordlist>::iterator it;
	for ( it=Dico.begin() ; it != Dico.end(); it++ ){ // Pour chacun des mots
	//	cout<<(*it).first<<endl;
		// Pour chacun des temps
		map <doubletemps,vector<vector<struct phoneme> > > :: iterator it_phonetisations;
	//	(*it).second.phonetisations
		for ( it_phonetisations = (*it).second.phonetisations.begin() ; it_phonetisations != (*it).second.phonetisations.end(); it_phonetisations++ ){
			//cout<<"\t"<<(*it_phonetisations).first.first<<" "<<(*it_phonetisations).first.second<<endl;
			//Pour chacunes de ces phonetisations a ce temps la on verifie les doublons !
			//(*it_phonetisations).second // le vecteur de vecteur
			remove_doubles_phonetisation_dans_vector((*it_phonetisations).second);
		}
		for ( it_phonetisations = (*it).second.phonetisations_sans_pause.begin() ; it_phonetisations != (*it).second.phonetisations_sans_pause.end(); it_phonetisations++ ){
			//cout<<"\t"<<(*it_phonetisations).first.first<<" "<<(*it_phonetisations).first.second<<endl;
			//Pour chacunes de ces phonetisations a ce temps la on verifie les doublons !
			//(*it_phonetisations).second // le vecteur de vecteur
			remove_doubles_phonetisation_dans_vector((*it_phonetisations).second);
		}
		
	}
}

static void ajoute_mot_et_phonetisation_aux_temps_t1_et_t2(string mot,vector<struct phoneme> new_phonetisation,unsigned short tps1,unsigned short tps2,bool pause){
	
	if(pause){ //on enleve la pause de la fin ... normalement !
		if(new_phonetisation[new_phonetisation.size()-1].pho.compare("-pau-")==0)
			new_phonetisation.pop_back();
			/* Et reaffectation du temps de fin qui change */
			tps2 = new_phonetisation[new_phonetisation.size()-1].trame + new_phonetisation[new_phonetisation.size()-1].duree;
	}
	
	if(DEBUG)cout<<"ajout du mot "<<mot<<" - aux temps "<< tps1<<" "<<tps2<<endl;
	
	/* RECHERCHE DU MOT DANS LE DICO */
	map<string,struct Wordlist>::iterator it;
	it = Dico.find(mot);
	if(it == Dico.end()){ //Si le mot n est pas dans le dico
		if(DEBUG)cout<<"NOT FIND"<<endl;
		// alors on l ajoute
		struct Wordlist new_word;
		new_word.word = mot;
		pair<map<string,struct Wordlist>::iterator,bool> ret = Dico.insert(pair<string,struct Wordlist>(mot,new_word));
		if (ret.second==false){
			cout << "element 'z' already existed"<<endl;
			cout << "C EST ANORMAL"<<endl;
			exit(0);
		}
		it = ret.first;
	}
	//else{} s 'il existe alors l iterator fait avec le find pointe sur le bon element, donc on l a
	
	/* On rajoute maintenant dans le vector la map de Wordlist les temps avec la phonetisation*/
	//	(*it).second; //la Wordlist
	
	/*constitution de la cle du mot*/
	doubletemps temps_mots(tps1,tps2);
	

	if(!pause){ // mode pause a enlever a la fin ou pas 
		
		//if(mot.compare("Malonne")==0){
		//	cout<<mot<<" : "<<tps1 << " - " << tps2 << endl; 	
		//}
		/* Est ce que ce mot dispose deja de ce temps ??? ou pas */		
		map <doubletemps,vector<vector<struct phoneme> > >::iterator it_temps = (*it).second.phonetisations.find(temps_mots);
		/* pas ces) temps deja presents*/
		if(it_temps == (*it).second.phonetisations.end()){
			if(DEBUG)cout<<mot<< "TEMPS VIDES !!"<<endl;
			vector <vector<struct phoneme> > A_Ajouter;
			A_Ajouter.push_back(new_phonetisation);
			(*it).second.phonetisations.insert(pair<doubletemps,vector <vector<struct phoneme> > >(temps_mots,A_Ajouter));
		}else{
			if(DEBUG)cout<< mot<< "TEMPS NON VIDES !!"<<endl;
			
			if(DEBUG)cout<<(*it_temps).second.size()<<endl;
		//	if(mot.compare("de1grader")==0)
				
			/*
			for(unsigned short p=0 ;p < (*it_temps).second.size();p++){
				cout<< (*it_temps).second[p][0].pho <<endl;
			}*/
			//if(phonetisation_exist_dans_vector(new_phonetisation,(*it_temps).second) == false)
			(*it_temps).second.push_back(new_phonetisation);
			
		}
	}else{ // le mot fini par une pause !
		map <doubletemps,vector<vector<struct phoneme> > >::iterator it_temps = (*it).second.phonetisations_sans_pause.find(temps_mots);
		/* pas ces) temps deja presents*/
		if(it_temps == (*it).second.phonetisations_sans_pause.end()){
			if(DEBUG)cout<<"TEMPS VIDES !!"<<endl;
			vector <vector<struct phoneme> > A_Ajouter;
			A_Ajouter.push_back(new_phonetisation);
			(*it).second.phonetisations_sans_pause.insert(pair<doubletemps,vector <vector<struct phoneme> > >(temps_mots,A_Ajouter));
		}else{
			if(DEBUG)cout<<"TEMPS NON VIDES !! "<<(*it_temps).second.size()<<endl;
			/* If faut tester si la meme phonetisation avec les memes score n existe pas*/
			//if(phonetisation_exist_dans_vector(new_phonetisation,(*it_temps).second) == false)
				(*it_temps).second.push_back(new_phonetisation);
			/*
			for(unsigned short p=0 ;p < (*it_temps).second.size();p++){
				cout<<p<<endl;	
			}*/
			if(DEBUG)cout<<(*it_temps).second.size()<<endl;
		}
	}	
}

static void loadPhonemesList(string phonemeFileName){
	
	vector<struct phoneme> suite_phoneme_tmp;
	string mot_temp;
	ifstream phonFile (phonemeFileName.c_str());
	unsigned short nb_ligne = 0 ;
	bool pause_en_fin = false;
	if (!phonFile.is_open())
		perror ("Error opening file");
	else
	{
		while (phonFile.good()) // lecture ligne a ligne
		{
			string tmp;
			getline(phonFile,tmp);
			/*DECOUPAGE DE LA LIGNE DE PHONETISATION*/
			vector<std::string> tab = split(tmp,' ');
			
			/** Si le mot est deja present dans la map "mot<->liste des phons" **/
			if(tab.size()>0){ // reverification sur la fin de fichier pour certains
				if(tab[0].compare("Mot:")==0){
					
					if(DEBUG)cout<<tab[0]<<" "<<tab[1]<<endl;
					/* On recupere les infos lues juste avant*/
					/* Sauf si c'est le debut et c'est vide - donc teste la taille du vecteur*/
					if(suite_phoneme_tmp.size()>0){
						
						/*Normalisation des scores car sinon ce sont la somme des scores */
						if(suite_phoneme_tmp.size()>1)
							normalisation_score_acoustique(suite_phoneme_tmp);
						unsigned short trame_debut_de_mot = suite_phoneme_tmp[0].trame;
						unsigned short trame_fin_de_mot   = suite_phoneme_tmp[suite_phoneme_tmp.size()-1].trame+suite_phoneme_tmp[suite_phoneme_tmp.size()-1].duree;
//						cout<<"trame de debut:"<<trame_debut_de_mot<<endl;
//						cout<<"trame de fin:"  <<trame_fin_de_mot<<endl;
						ajoute_mot_et_phonetisation_aux_temps_t1_et_t2(mot_temp,suite_phoneme_tmp,trame_debut_de_mot,trame_fin_de_mot,false);
						if(pause_en_fin && suite_phoneme_tmp.size()>1)
							ajoute_mot_et_phonetisation_aux_temps_t1_et_t2(mot_temp,suite_phoneme_tmp,trame_debut_de_mot,trame_fin_de_mot,true);
					}
					
					/*
					Dans tous les cas, On efface les infos du mot precedent
					
					et on reinitialise la nouvelle structure qui contient les phonemes
					*/
					pause_en_fin = false;
					suite_phoneme_tmp.clear();
					mot_temp = tab[1];
					
				}else{ // on remplit alors les phonetisations du mot
					if(DEBUG)cout<<"\t"<<tab[0]<<" "<<tab[1]<<endl;
					if(tab.size() != 2){
						cout<< tmp <<endl;
						cout<<"\t"<<tab[0]<<" <-> "<<tab[1]<<endl;
						cerr<< "Erreur dans le fichier phons a la ligne:"<< nb_ligne<< endl;
						exit(-1);
					}
					if(tab[0].compare("pause") == 0 ){
						tab[0]="-pau-";
					}
					struct phoneme new_pho = load_info_phoneme(tab[0],tab[1]);
					suite_phoneme_tmp.push_back(new_pho); // ajout du phonemene dans la liste des phonemes de la phonetisation
					if(new_pho.pho.compare("-pau-")==0){pause_en_fin=true;}
					if(DEBUG)cout<<new_pho.pho<<" "<<new_pho.score<<" "<<new_pho.trame<<" "<<new_pho.duree<<" "<<endl;
				}
			}
			nb_ligne++;
		}//Fin du while phonFile.good
		/* IL FAUT AJOUTER LE DERNIER MOT VU */
		if(suite_phoneme_tmp.size()>0){
			if(suite_phoneme_tmp.size()>1)
				normalisation_score_acoustique(suite_phoneme_tmp);
			unsigned short trame_debut_de_mot = suite_phoneme_tmp[0].trame;
			unsigned short trame_fin_de_mot   = suite_phoneme_tmp[suite_phoneme_tmp.size()-1].trame+suite_phoneme_tmp[suite_phoneme_tmp.size()-1].duree;

			ajoute_mot_et_phonetisation_aux_temps_t1_et_t2(mot_temp,suite_phoneme_tmp,trame_debut_de_mot,trame_fin_de_mot,false);
			if(pause_en_fin && suite_phoneme_tmp.size()>1)
				ajoute_mot_et_phonetisation_aux_temps_t1_et_t2(mot_temp,suite_phoneme_tmp,trame_debut_de_mot,trame_fin_de_mot,true);
		}
		
	}
}

/********* CHARGEMENT DU TREILLIS ************/
struct I_info{
	unsigned int node_number;
	float node_time;
};

struct J_info{
	unsigned int link_number;
	unsigned int start;
	unsigned int end;
	string word;
	float value;
	float acoustique_value;
};

vector<string> header;
vector<struct I_info> I;
vector<struct J_info> J;

static void loadLattice(string lattice_file_name){
	

	ifstream LatticeFile (lattice_file_name.c_str());
	if (!LatticeFile.is_open())
		perror ("Error opening file");
	else
	{
		while (LatticeFile.good()) // lecture ligne a ligne
		{
			string tmp;
			getline(LatticeFile,tmp);
			vector<std::string> tab = split(tmp,'=');
			if(tab.size()){
				bool i_or_j = false;
				if(tab[0].compare("I")==0){
					i_or_j = true;
					vector<std::string> tab = split(tmp,'\t');
					vector<std::string> node = split(tab[0],'=');
					vector<std::string> time = split(tab[1],'=');
					struct I_info new_I;
					new_I.node_number = atoi(node[1].c_str());
					new_I.node_time   = atof(time[1].c_str()) *100;
					I.push_back(new_I);
				}
				if(tab[0].compare("J")==0){
					i_or_j = true;
					vector<std::string> tab = split(tmp,'\t');
					vector<std::string> l_number = split(tab[0],'=');
					vector<std::string> l_start  = split(tab[1],'=');
					vector<std::string> l_end    = split(tab[2],'=');
					vector<std::string> l_word   = split(tab[3],'=');
					vector<std::string> l_value  = split(tab[4],'=');
					vector<std::string> l_ac_va  = split(tab[5],'=');
					struct J_info new_J;
					
					new_J.link_number			= atoi(l_number[1].c_str());

					

						
					new_J.start				= atoi(l_start[1].c_str());
					if(l_start[1].compare("-1")==0){ // PATCH SI UN LINK DEBUTE EN MOINS 1 (-1) ....BUG DANS SPEERAL
						new_J.start			= 0;
					}
					new_J.end				= atoi(l_end[1].c_str());
					new_J.word				= l_word[1];
					new_J.value				= atof(l_value[1].c_str());
					new_J.acoustique_value	= atof(l_ac_va[1].c_str());
					//cout<<"J="<< l_start[1] << " "<<new_J.start<<endl;
					if(l_start[1].compare("-1")!=0){
						J.push_back(new_J);
					}
				}
				if(!i_or_j){ //C EST LES HEADERS OU LES AUTRE TRUCS.....ON S EN FOU POUR L INSTANT
					if(tab[0].compare("N")!=0 && tmp.substr(0,1).compare("#")!=0){
						//cout<<"header:"<<tmp<<endl;
						header.push_back(tmp);
					}
				}
			}
		}
	}
	//cout<<I.size()<<endl;
	//cout<<J.size()<<endl;
	temps_de_debut = I[0].node_time;
}

static void debug_lattice(){
	map<string,struct Wordlist>::iterator it;
	string Malonne ("Malonne");
	it = Dico.find(Malonne);
	if(it == Dico.end()){
		cout<< "Eyll pas trouve !!"<<endl;	
	}else{
		cout<<"Taille "<<(*it).second.phonetisations.size() <<endl;
		cout<<"Taille "<<(*it).second.phonetisations_sans_pause.size() <<endl;
		
		map <doubletemps,vector<vector<struct phoneme> > > :: iterator iter_tps_phos;
		
		for ( iter_tps_phos=(*it).second.phonetisations_sans_pause.begin() ; iter_tps_phos != (*it).second.phonetisations_sans_pause.end(); iter_tps_phos++ )
		    cout << (*iter_tps_phos).first.first << " => " << (*iter_tps_phos).first.second << endl;

	//	(*it).second.phonetisations_sans_pause;
	}
	exit(0);
}

/*
struct Wordlist{
	string word;
	map <doubletemps,vector<vector<struct phoneme> > > phonetisations;  // un temps m tps de debut et de fin peut contenir plusieurs phonetisations ! donc vecteurs de vectors 
	map <doubletemps,vector<vector<struct phoneme> > > phonetisations_sans_pause;
};

struct phoneme{
	string pho;
	float score;
	unsigned short trame;
	unsigned short duree;
};

*/

vector<struct I_info> I_final;
vector<struct J_info> J_final;

static void transforme_lattice(){
	I_final=I;

	//J_final=J;
	map<string,struct Wordlist>::iterator it;
	map <doubletemps,vector<vector<struct phoneme> > >::iterator liste_des_phonetisations; 
	//debug_lattice();

	for(unsigned int j_link=0;j_link<J.size();j_link++){
	
		unsigned int start = I[J[j_link].start].node_time - I[0].node_time;
		unsigned int end   = I[J[j_link].end].node_time   - I[0].node_time;
		
		bool trouve=false;
		bool trouve_sans_pause=false;
		
		if(start==0){start=1;} //PATCH CAR CERTAINS MOTS COMMENCENTS A 1 ET PAS A 0 DANS LES TREILLIS
		
		unsigned int end_good  = end; 
		it = Dico.find(J[j_link].word);
		if(it == Dico.end() && J[j_link].word.compare("-pau-") !=0 ){
			cerr << J[j_link].word <<" PAS TROUVE !!!"<<endl;
			exit(0);
		}else{
			doubletemps a_chercher(start,end);
			//cout << " J Pas " << j_link << endl;

			liste_des_phonetisations = (*it).second.phonetisations.find(a_chercher); // recherche en 1er dans la liste normale
			if(liste_des_phonetisations == (*it).second.phonetisations.end() && J[j_link].word.compare("-pau-") !=0){
				;
			}else{trouve=true;}
			
			if(trouve==false){ // si je ne trouve pas
				liste_des_phonetisations = (*it).second.phonetisations_sans_pause.find(a_chercher); // recherche dans la liste sans les pauses !
				if(liste_des_phonetisations == (*it).second.phonetisations_sans_pause.end() && J[j_link].word.compare("-pau-") !=0){
					;
				}else{trouve=true;trouve_sans_pause=true;}
			}
			if(!trouve){ //PATCH CAR CERTAINS MOTS FINISSENT 2 TRAMES EN MOINS DANS LES TREILLIS....
				end_good=end-2;
				a_chercher = make_pair (start,end-2);
				liste_des_phonetisations = (*it).second.phonetisations.find(a_chercher);
				if(liste_des_phonetisations == (*it).second.phonetisations.end() && J[j_link].word.compare("-pau-") !=0){
					;
				}else{trouve=true;}
			}
			if(!trouve){ //PATCH CAR CERTAINS MOTS FINISSENT 2 TRAMES EN MOINS DANS LES TREILLIS....
				end_good=end-1;
				a_chercher = make_pair (start,end-1);
				liste_des_phonetisations = (*it).second.phonetisations.find(a_chercher);
				if(liste_des_phonetisations == (*it).second.phonetisations.end() && J[j_link].word.compare("-pau-") !=0){
					;
				}else{trouve=true;}
			}
			if(!trouve){
				cerr<<"\tERREUR DANS: "<<file_out<<" - pas de mot "<< J[j_link].word <<" en (" << start <<"," << end_good << ")"<<endl;
				exit(0);
			}else{
				//cout << "NODE "<< J[j_link].word<< "::" << J[j_link].start << " -> " << J[j_link].end     << endl;
				//cout << "\tOK : " << a_chercher.first <<  " "  << a_chercher.second << " "<< (*liste_des_phonetisations).second.size() << endl;
				/* AFFICHAGE DES NOUVEAUX NOEUDS MAIS ON GARDE CELUI DE DEBUT DE CELUI DE FIN*/
			
				//map<string,struct Wordlist> :: iterator it_word = Dico.find(J[j_link].word);
				unsigned int last_end_node = -1;
				if( J[j_link].word.compare("-pau-") == 0){
					J_final.push_back(J[j_link]);
				}
				if( J[j_link].word.compare("-pau-") != 0){
					for(unsigned short i=0;i<(*liste_des_phonetisations).second.size();i++){
						for(unsigned short p=0 ; p<(*liste_des_phonetisations).second[i].size();p++){
							//cout<< (*liste_des_phonetisations).second[i][p].pho <<endl;
							J_info new_link;

							if((*liste_des_phonetisations).second[i].size() == 1){ // un seul phoneme
								new_link.start = J[j_link].start;
								new_link.end   = J[j_link].end;
								new_link.word  = (*liste_des_phonetisations).second[i][p].pho;
								new_link.value = 0;
								new_link.acoustique_value = (*liste_des_phonetisations).second[i][p].score;
								J_final.push_back(new_link);
								//if(last_end_node>100000)
								//	cout << "\tu1 S=" << new_link.start << " E=" << new_link.end << " " << new_link.word<< endl;	
							}else
							if(p==0){ /* On ne change pas le debut */
								//cout<<"debut donc noeud de depart "<< J[j_link].start <<endl;
								new_link.start = J[j_link].start;
								new_link.end   = I_final.size();
								last_end_node  = new_link.end;
								new_link.word  = (*liste_des_phonetisations).second[i][p].pho;
								new_link.value = 0;
								new_link.acoustique_value = (*liste_des_phonetisations).second[i][p].score;
								J_final.push_back(new_link);
								//if(last_end_node>100000)
								//		cout << "\tu2 S=" << new_link.start << " E=" << new_link.end << " " << new_link.word<< endl;	
								/* Ajout d un noeud*/
								I_info new_node;
								new_node.node_number = I_final.size();
								new_node.node_time   = (I[0].node_time + (*liste_des_phonetisations).second[i][p].trame + (*liste_des_phonetisations).second[i][p].duree);
								I_final.push_back(new_node);
								//cout << "\td S=" << new_link.start << " E=" << new_link.end << " " << new_link.word<< endl;
								//cout << "\tN I=" <<new_link.end<<"\t"<<"t="<< new_node.node_time <<endl;// " " << I[0].node_time << " " << start << " "<< (*liste_des_phonetisations).second[i][p].trame << " "<< (*liste_des_phonetisations).second[i][p].duree <<endl;
							}else
							if(p==(*liste_des_phonetisations).second[i].size()-1){ /* on ne change pas la fin*/
								//cout<<"fin donc noeud de fin "<< J[j_link].end <<endl;
								new_link.start = last_end_node;
								new_link.end   = J[j_link].end;
								new_link.word  = (*liste_des_phonetisations).second[i][p].pho;
								new_link.value = 0;
								new_link.acoustique_value = (*liste_des_phonetisations).second[i][p].score;
								J_final.push_back(new_link);
								//if(last_end_node>100000)
								//	cout << "\tu3 S=" << new_link.start << " E=" << new_link.end << " " << new_link.word<< endl;	
	
								//cout << "\tf S=" << new_link.start << " E=" << new_link.end << " " << new_link.word<< endl;
								/* Ajout d un noeud*/
								//I_info new_node;
								//new_node.node_number = I.size();
								//new_node.node_time   = J[j_link].end;
							}
							else{
								//cout<<"noeud normal ajout du nouveazu noeud de fin"<<endl;
								new_link.start = last_end_node;
								new_link.end   = I_final.size();
								last_end_node  = new_link.end;
								new_link.word  = (*liste_des_phonetisations).second[i][p].pho;
								new_link.value = 0;
								new_link.acoustique_value = (*liste_des_phonetisations).second[i][p].score;
								J_final.push_back(new_link);
								//if(last_end_node>100000)
								//	cout << "\tu4 S=" << new_link.start << " E=" << new_link.end << " " << new_link.word<< endl;	
		
								/* Ajout d un noeud*/
								I_info new_node;
								new_node.node_number = I_final.size();
								new_node.node_time   = (I[0].node_time + (*liste_des_phonetisations).second[i][p].trame + (*liste_des_phonetisations).second[i][p].duree);
								I_final.push_back(new_node);
								//cout << "\tm S=" << new_link.start << " E=" << new_link.end << " " << new_link.word<< endl;
								//cout << "\tN I=" <<new_link.end<<"\t"<<"t="<< new_node.node_time <<endl;
							}
						}
					}
				}
			}
		}
	}
}

/*** ***/
/*

struct I_info{
	unsigned int node_number;
	float node_time;
};

struct J_info{
	unsigned int link_number;
	unsigned int start;
	unsigned int end;
	string word;
	float value;
	float acoustique_value;
};

vector<string> header;
vector<struct I_info> I;
vector<struct J_info> J;


map<string,struct Wordlist> Dico;

struct Wordlist{
	string word;
	map <doubletemps,vector<vector<struct phoneme> > > phonetisations;  // un temps m tps de debut et de fin peut contenir plusieurs phonetisations ! donc vecteurs de vectors 
	map <doubletemps,vector<vector<struct phoneme> > > phonetisations_sans_pause;
};

struct phoneme{
	string pho;
	float score;
	unsigned short trame;
	unsigned short duree;
};

*/

static void affichage_dico_mot(){
	map<string,struct Wordlist>::iterator it;
	map <doubletemps,vector<vector<struct phoneme> > >::iterator liste_des_phonetisations; 

	for(it=Dico.begin() ; it != Dico.end() ; it++){
		cout<<(*it).first<<endl;
		//Wordlist //(*it).second
		/*phonetisations avec pause */
		for(liste_des_phonetisations=(*it).second.phonetisations.begin() ;
					liste_des_phonetisations != (*it).second.phonetisations.end() ;
							liste_des_phonetisations++){
			cout<<"\t"<< (*liste_des_phonetisations).first.first <<" "<< (*liste_des_phonetisations).first.second <<endl;
			for(unsigned short p=0 ;p < (*liste_des_phonetisations).second.size() ;p++){
				cout<<"\t";
				for(unsigned short p2=0 ;p2 < (*liste_des_phonetisations).second[p].size() ;p2++){
					cout<<"("<<(*liste_des_phonetisations).second[p][p2].pho<<" "<<(*liste_des_phonetisations).second[p][p2].trame<<" "<<(*liste_des_phonetisations).second[p][p2].duree<<" "<<(*liste_des_phonetisations).second[p][p2].score<<") ";
				}
				cout<<endl;
			}
			//if((*liste_des_phonetisations).second.size()>1){ exit(0);}
		}
		/*phonetisations sans pause */
		for(liste_des_phonetisations=(*it).second.phonetisations_sans_pause.begin() ;
					liste_des_phonetisations != (*it).second.phonetisations_sans_pause.end() ;
							liste_des_phonetisations++){
			cout<<"\t"<< (*liste_des_phonetisations).first.first <<" "<< (*liste_des_phonetisations).first.second <<endl;
			
			for(unsigned short p=0 ;p < (*liste_des_phonetisations).second.size() ;p++){
				cout<<"\t";
				for(unsigned short p2=0 ;p2 < (*liste_des_phonetisations).second[p].size() ;p2++){
					cout<<"("<<(*liste_des_phonetisations).second[p][p2].pho<<" "<<(*liste_des_phonetisations).second[p][p2].trame<<" "<<(*liste_des_phonetisations).second[p][p2].duree<<" "<<(*liste_des_phonetisations).second[p][p2].score<<") ";
				}
				cout<<endl;
			}
			//if((*liste_des_phonetisations).second.size()>1){ exit(0);}
		}
	}
}

static void ecrit_treillis_phonemes(string sortie){
	ofstream outfile (sortie.c_str());
	/*ENTETE*/
	//string header("VERSION=0.1\nUTTERANCE=20041008_1800_1830_INFO_DGA\nbase=10\nlmscale=10.000000\nwdpenalty=-14.000000\n");
	
	
	for(unsigned int h=0;h<header.size();h++){
		outfile.write(header[h].c_str(),header[h].size());
		outfile.write("\n",1);
	}
	stringstream i_s; i_s << I_final.size();
	string iss (i_s.str());
	stringstream j_s; j_s << J_final.size();
	string jss (j_s.str());
	 
	string out("N="+iss+"\t"+"L="+jss+"\n");
	outfile.write(out.c_str(),out.size());
	/* I= */
	for(unsigned int i=0;i<I_final.size();i++){
	 	stringstream num_s; num_s << I_final[i].node_number;
	 	string num (num_s.str());
	 	stringstream time_s; time_s << (I_final[i].node_time/100);
	 	string time (time_s.str());
		string out("I=" + num + "\tt=" + time+"\n");
		outfile.write(out.c_str(),out.size());
	 }
	 for(unsigned int j=0;j<J_final.size();j++){
	 	stringstream num_s; num_s << j;
	 	string num (num_s.str());
	 	stringstream start; start<<J_final[j].start;
	 	string start_s(start.str());
	 	stringstream end; end<<J_final[j].end;
	 	string end_s(end.str());
	 	stringstream valeur; valeur<<J_final[j].value;
	 	string valeur_s(valeur.str());
	 	stringstream acoustic; acoustic<<J_final[j].acoustique_value;
	 	string acoustic_s(acoustic.str());
		
	 	string mot_s = J_final[j].word;

		string out("J=" + num + "\tS="+start_s + "\tE="+end_s +"\tW="+ mot_s +"\tv="+acoustic_s + "\tp="+acoustic_s +"\n");
		//if(j>47089)
		//	cout <<out;
		
//		string out("J=" + num + "\tS="+start_s + "\tE="+end_s +"\tW=<token id=\"0\" in=\""+ mot_s +"\" /> \tv="+valeur_s + "\ta="+acoustic_s +"\n");
		outfile.write(out.c_str(),out.size());
	 }
	 outfile.close();
}

void print_help(){
	cout<<"Fusion_phon_N_treillis V2.1"<<endl;
	cout<<" - debug du numero start du noeud"<<endl;
	cout<<" - ajout score posterior"<<endl;
	cout<<"	gregory.senay@univ-avignon.fr"<<endl;
	cout<<" <i:PhonsListe> <i:Trellis> <o:phol>"<<endl;	
}

int main(int argc, char ** argv){
	

	if(argc == 4){
		for(int a=0;a<3;a++)
		if( strcmp(argv[a],"-h") == 0 || strcmp(argv[a],"--help") == 0 || strcmp(argv[a],"-help") == 0 ){
			print_help();
			exit(0);
		}
		file_out = argv[3];
		loadPhonemesList(argv[1]);
		//enleve_doublons();
		loadLattice(argv[2]);
			
		transforme_lattice();
		//affichage_dico_mot();
		ecrit_treillis_phonemes(argv[3]);
		cout << file_out << " -> done"<<endl;
	}else{
		print_help();
	}
	
	return 0;
}
