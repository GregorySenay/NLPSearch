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

#ifndef ADD_H_fusion_phon_N_treillis
#define ADD_H_fusion_phon_N_treillis

int add(int a, int b);

bool compare_phoneme(struct phoneme a,struct phoneme b)
bool compare_vector_phonemes(vector<struct phoneme>va,vector<struct phoneme> vb)
bool phonetisation_exist_dans_vector(vector<struct phoneme> a,vector <vector<struct phoneme> > vec_b)
void remove_doubles_phonetisation_dans_vector(vector <vector<struct phoneme> > & vec_b)
doubletemps_N_num cree_double_temps(unsigned short a,unsigned short b,unsigned short c)
struct phoneme load_info_phoneme(string phon_name, string info2)
void normalisation_score_acoustique(vector<struct phoneme> & suite_phoneme_tmp) // Pour enlever le score cumule
void enleve_doublons()
void ajoute_mot_et_phonetisation_aux_temps_t1_et_t2(string mot,vector<struct phoneme> new_phonetisation,unsigned short tps1,unsigned short tps2,bool pause)
void loadPhonemesList(string phonemeFileName)
void loadLattice(string lattice_file_name)
void debug_lattice()
void transforme_lattice()
void affichage_dico_mot()
void ecrit_treillis_phonemes(string sortie)
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) 
std::vector<std::string> split(const std::string &s, char delim)

#endif
