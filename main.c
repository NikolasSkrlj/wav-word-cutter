#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <string.h>
#include <io.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#define		BUFFER_LEN	4096
#define MAXS 1000

//struktura koja sadrzi rijeci i njihove pocetke i krajeve u sekundama(ili sampleovima *16000);
typedef struct {
    char word[20];
    double start;
    double finish;
}words;

//struktura koja sadrzi putanje datoteka i raw naziv
typedef struct {
    char lab[128];
    char txt[128];
    char wav[128];
    char raw_prefix[128];

}files;

static void encode_file (const char *infilename, const char *outfilename, int filetype){

    static float buffer [BUFFER_LEN] ;

	SNDFILE		*infile, *outfile ;
	SF_INFO		sfinfo ;
	int			k, readcount ;

	printf ("    %s -> %s ", infilename, outfilename) ;
	fflush (stdout) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
	{	printf ("Error : could not open file : %s\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		}

	sfinfo.format = filetype ;

	if (! sf_format_check (&sfinfo))
	{	sf_close (infile) ;
		printf ("Invalid encoding\n") ;
		return ;
		} ;

	if (! (outfile = sf_open (outfilename, SFM_WRITE, &sfinfo)))
	{	printf ("Error : could not open file : %s\n", outfilename) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	while ((readcount = sf_read_float (infile, buffer, BUFFER_LEN)) > 0)
		sf_write_float (outfile, buffer, readcount) ;

	sf_close (infile) ;
	sf_close (outfile) ;

	printf ("ok\n") ;

	return ;
}

int getWords(char *base, words tmp[])
{
	int n=0,i,j=0;

	for(i=0;;i++)
	{
		if(base[i]!=' '){
		    //rjesavanje zagrada u .txt fileu
            if(base[i]=='>' || base[i]=='<')  continue;

            //zamjena hrvatskih slova u txt file-u
            if(base[i]=='{'){
                tmp[n].word[j++]='s';
            } else if(base[i]=='}'){
                tmp[n].word[j++]='d';
            } else if(base[i]=='^'){
                tmp[n].word[j++]='c';

            } else if(base[i]=='~'){
                tmp[n].word[j++]='c';
            }else if(base[i]=='`'){
               tmp[n].word[j++]='z';
            }else {
                 tmp[n].word[j++]=base[i];
            }


		}
		else{
			tmp[n].word[j++]='\0';//insert NULL
			tmp[n].start = 0;
			tmp[n].finish = 0;
			n++;
			j=0;
		}
		if(base[i]=='\0')
		    break;
	}
	return n;

}

void make_directory(const char* name) {
       _mkdir(name);

   }

//funkcija po kojoj se azurira ime datoteke ako vec ista postoji
int doubleWords(words collection[], char* target, int wordnmbr){
    int i, cnt = 0;

    for(i = wordnmbr; i >=0; i--){
        if(strcmp(collection[i].word, target) == 0){
            cnt++;
        }
    }
    return cnt;
}

//funkcija koja kontrolira nepravilnosti u hrvatskom govoru
int dropLetters(char* word){
    int i, cnt = 0;

    for(i = 0; i < strlen(word);i++){
        if(word[i]=='l'){
            if(word[i+1]=='j'){
                cnt++;
            }
        }

        if(word[i]=='n'){
            if(word[i+1]=='j'){
                cnt++;
            }
        }
        if(word[i]=='t'){
            if(word[i+1]=='s'){
                cnt++;
            }
        }

        if(word[i]=='d'){
            if(word[i+1]=='s'){
                cnt++;
            }
        }
        if(word[i]=='t'){
            if(word[i+1]=='c'){
                cnt++;
            }
        }

        if(word[i]=='d'){
            if(word[i+1]=='c'){
                cnt++;
            }
        }
        if(word[i]=='i'){
            if(word[i+1]=='j'){
                cnt++;
            }
        }

    }
    return cnt;

}
int listFiles(const char *path, files* tmp)
{
    struct dirent *dp;
    DIR *dir = opendir(path);

    // Unable to open directory stream
    if (!dir)
        return;
    int i = 0;

    //iteriranje po svim datotekama u wav folderu dok god ih ima
    while ((dp = readdir(dir)) != NULL){

        if ( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ){

        } else {

            //postavljanje nazive i putanje datoteka u polje struktura
            memset (&tmp[i].wav, 0, sizeof (tmp[i].wav));
            memset (&tmp[i].lab, 0, sizeof (tmp[i].lab));
            memset (&tmp[i].txt, 0, sizeof (tmp[i].txt));
            memset (&tmp[i].raw_prefix, 0, sizeof (tmp[i].raw_prefix));

            strcpy(tmp[i].lab, "C:\\Users\\Nikolas\\Desktop\\Projekt_prog1\\lab_sm0405\\");
            strcpy(tmp[i].txt, "C:\\Users\\Nikolas\\Desktop\\Projekt_prog1\\txt_sm0405\\");
            strcpy(tmp[i].wav, "C:\\Users\\Nikolas\\Desktop\\Projekt_prog1\\wav_sm0405\\");

            char txttmp[64];
             memset (&txttmp, 0, sizeof (txttmp));
            strncpy(txttmp, dp->d_name, strlen(dp->d_name)-4);
            strcat(txttmp, ".txt\0");
            strcat(tmp[i].txt, txttmp);

            strcat(tmp[i].wav, dp->d_name);

            char labtmp[64];
             memset (&labtmp, 0, sizeof (labtmp));
            strncpy(labtmp, dp->d_name, strlen(dp->d_name)-4);
            strcat(labtmp, ".lab\0");
            strcat(tmp[i].lab, labtmp);

            char rawtmp[64];
             memset (&rawtmp, 0, sizeof (rawtmp));
            strncpy(tmp[i].raw_prefix, dp->d_name, strlen(dp->d_name)-4);


            i++;
        }
    }

    // Close directory stream
    closedir(dir);
    return i;
}


int main()
{
    FILE *lab_file = NULL, *txt_file = NULL;
    words collection[50];
    files workfiles[50];

    double start[500];
    double end[500];
    char letter[500][20];
    int i = 0;
    char line[256];


    //ubacivanje putanja svih datoteka u polje struktura workfiles
    char path[100];
     memset (&path, 0, sizeof (path)) ;
    strcpy(path, "C:\\Users\\Nikolas\\Desktop\\wav_sm0405");
    int filembr = listFiles(path, workfiles);

    //iteracija po datotekama
    for(int j = 0; j < filembr; j++){

        //printf("%s\n%s\n%s\n%s\n\n", workfiles[j].wav, workfiles[j].lab, workfiles[j].txt, workfiles[j].raw_prefix) ;

        const char* inFileName = (const char*)workfiles[j].wav ;
        const char* lab_namepath = (const char*)workfiles[j].lab;
        const char* txt_path = (const char*)workfiles[j].txt;

        char rawfilename[128];
        char temp[123];


        //odreðivanje putanje raw datoteke i spremanje u poseban folder
        memset (&rawfilename, 0, sizeof (rawfilename));
        memset (&temp, 0, sizeof (temp));

        strcpy(temp, workfiles[j].raw_prefix);
        strcat(temp, "raw_file.wav");

        strcpy(rawfilename, "C:\\Users\\Nikolas\\Desktop\\Projekt_prog1\\raw_files\\");
        strcat(rawfilename, temp);

        make_directory("C:\\Users\\Nikolas\\Desktop\\Projekt_prog1\\raw_files\\");


        // dekodiranje u raw format
        encode_file (inFileName, rawfilename, SF_FORMAT_RAW |  SF_FORMAT_PCM_16) ;

        txt_file = fopen(txt_path, "r");

        if (txt_file == NULL) {
            perror("Failed: ");
            return 1;
        }

        //uzimamo cijelu liniju i spremamo je u char array
        fscanf(txt_file, " %[^\n] ", line);
        //printf("%s\n", line);

        //rastavljamo liniju na rijeci i spremamo u polje struktura pripremajuci za azuriranje trajanja rijeci
        int wordsNmbr;
        wordsNmbr = getWords(line, collection) + 1;


        //otvaranje lab datoteke kako bi azurirali trajanja rijeci
        lab_file = fopen(lab_namepath, "r");
        if (lab_file == NULL) {
            perror("Failed: ");
            return 1;
        }

        //spremanje iz datoteke u polja iste velicine kako bi dobili trajanje svakog slova u rijeci
        while(fscanf(lab_file, " %lf %lf %s", &start[i], &end[i], letter[i]) >=0){
                i++;
        }

        //indeks u lab datoteci -> pocinje od 1 jer je prva tisina sil
        int tmp = 1;

        //ispunjavanje polja struktura sa informacijama o rijecima, pocetcima i krajevima
        //podatke o slovima citamo iz polja u koje smo prije ucitali podatke iz .lab datoteke
        for(int i = 0; i < wordsNmbr; i++){

                //kontrola ako se umjesto rijeci pojavi uzdah ili tisina(sil)
                if(strcmp(letter[tmp], "uzdah") == 0){
                    collection[i].start = start[tmp]/10000000;
                    collection[i].finish = end[tmp]/10000000;
                    tmp++;
                    printf("Rijec je %s, pocetak je %lf a kraj je %lf\n", collection[i].word, collection[i].start, collection[i].finish);

                    continue;

                }
                if(strcmp(letter[tmp], "sil") == 0){
                    collection[i].start = start[tmp]/10000000;
                    collection[i].finish = end[tmp]/10000000;
                    tmp++;
                    printf("Rijec je %s, pocetak je %lf a kraj je %lf\n", collection[i].word, collection[i].start, collection[i].finish);

                    continue;
                }

            int len = strlen(collection[i].word);

            //provjeravanje nepravilnosti u govoru te azuriranje indeksa prema rezultatu
            collection[i].start = start[tmp]/10000000;
            int check = dropLetters(collection[i].word);

            tmp -= check ;
            collection[i].finish = end[tmp + len-1]/10000000; // kraj je pocetak plus duljina rijeci
            tmp += len ;
            printf("Rijec je %s, pocetak je %lf a kraj je %lf\n", collection[i].word, collection[i].start, collection[i].finish);

        }

        static double data [BUFFER_LEN];
        SF_INFO		sfinfo1, sfinfo2 ;
        SNDFILE	*infile, *outfile ;

        const char	*outfilename;

        memset (&sfinfo1, 0, sizeof (sfinfo1)) ;
        memset (&sfinfo2, 0, sizeof (sfinfo2)) ;

        //printf("Samplerate je %d\n, kanala ima %d \n", sfinfo.samplerate, sfinfo.channels);

        //za otvaranje raw file-a treba se zadat header info prije (sfinfo1)
        sfinfo1.samplerate	= 16000 ;
        sfinfo1.format		= SF_FORMAT_RAW | SF_FORMAT_PCM_16;
        sfinfo1.channels	= 1 ;

        if (! (infile = sf_open (rawfilename, SFM_READ, &sfinfo1)))
        {	printf ("Error : could not open file : %s\n", rawfilename) ;
            puts (sf_strerror (NULL)) ;
            exit (1) ;
            }

        double limit, tail;
        double readcount;
        /*
                    SAMPLE_COUNT	(SAMPLE_RATE * duration)
                    sveukupno ima => 4.58 s * 16000 = 73 280 sampleova
                    rijec dobar traje od 0 s - 0.336 s
                    0.336 * 16000 => rijec zavrsava sa 5376-im sampleom
                    citamo frameove dok god ne dodjemo do 5376-og i onda brakeamo
        */

        //izrada direktorija za testni slucaj
         char  directory[128];
         memset (&directory, 0, sizeof (directory)) ;
         strcpy(directory, workfiles[j].raw_prefix);

        make_directory(strncat(directory, "\\\\", 2));

        //iteriranje po rijecima, koristeci pocetke i krajeve iz struktura odabiremo granice "rezanja" audio datoteke
        for(int i = 0; i < wordsNmbr; i++){

                limit = collection[i].start * 16000;
                tail = collection[i].finish *16000;

                //provjeravanje za prethodne pojave rijeci
                char tmp[20];
                memset (&tmp, 0, sizeof (tmp)) ;
                int sufix = doubleWords(collection, collection[i].word, i);

                if(sufix > 1){
                    char suf[2];
                    suf[0] = sufix + 48;
                    suf[1] ='\0';

                    strcpy(tmp, collection[i].word);
                    strcat(tmp, suf);
                } else {
                    strcpy(tmp, collection[i].word);
                }

                //uredjivanje direktorija i imena datoteke

                char base[100];
                memset (&base, 0, sizeof (base)) ;
                strcpy(base, "C:\\Users\\Nikolas\\Desktop\\Projekt_prog1\\");
                outfilename= strcat(base, directory);
                outfilename = strcat(base, tmp);
                strncat(outfilename, ".wav", 4);

                //podesavanje file cursora na odredjeni segment
                memset (&data, 0, sizeof (data)) ;
                sf_seek(infile, (sf_count_t)limit, SEEK_SET);

                //podesavanje zaglavlja za nove datoteke
                sfinfo2.samplerate	= 16000 ;
                sfinfo2.format		= SF_FORMAT_WAV | SF_FORMAT_PCM_16;
                sfinfo2.channels	= 1 ;

                //otvaranje nove datoteke
                 if (! (outfile = sf_open (outfilename, SFM_WRITE, &sfinfo2)))
                        {	printf ("Error : could not open file : %s\n", outfilename) ;
                            puts (sf_strerror (NULL)) ;
                            exit (1) ;
                            } ;

                //read/write
                while ((readcount = sf_read_double (infile, data, BUFFER_LEN)) > 0){
                        //limit uvodimo jer readcount je nakon svake iteracije max 4096 a nama treba sveukupno
                        limit += readcount;


                    if(limit < tail){

                        //dok je broj ucitanih podataka manji od kraja, zapisuj podatke
                        sf_write_double (outfile, data, readcount);

                    } else {
                        //zapisi zadnji ostatak podataka koji je manji od readcount-a (4096)
                        int tmp = limit - 4096;
                        sf_write_double (outfile, data, tail - tmp);
                        sf_close (outfile) ;

                        break;

                    }
                }
        }
		
        sf_close(outfile);
        sf_close (infile) ;

        //resetiranje indeksa za sljedece datoteke
        i = 0;
        tmp = 1;
    }

	return 0 ;

}
