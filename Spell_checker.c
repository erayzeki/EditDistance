#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>




struct WORD{
	char wrong[20];
	char correct[20];
};

static char smallDict[997][20]; // temel hashtable
static struct WORD wrongTable[997]; // hatali kelimelerin hashtable'ý
static char cumleKelimeler[50][20]; // kullanicidan alinan cümleler kelimelere ayrýlarak tutulur
static int kelimeSayisi = 0; // cumleKelimeler dizisine eklenecek keline bu indexe eklenir
static char onerilenKelimeler[997][20]; // onerilecek kelimeler burada tutulur.
static int onerilenindex = 0; // onerilen kelimelerin indexi burada tutulur.


// kelimeleri sayiya cevirmek icin kullanilan horner fonksiyonu
int horner (char *word, int M){
	int R = 31; // horner metodu sabiti
	int len = strlen(word); // string uzunlugu
	
	int i;
	int key = 0; // donulecek anahtar
	for (i = 0; i <= len-1; i++){
		key += ((int)pow((double)R, (double)len-1-i)*(word[i]-'a'+1));
		key = key % M; // her adýmda mod alýnarak sayýnýn cok buyumesi engellenir
	}
	return key;
}


// dosyadan kelimeleri okuyup hashtable'a kaydeden fonksiyon
void loadTable(const char *filename){
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL){
		printf("Dosya acilamadi!");
	}
	
	char buffer[20]; // kelimelerin okunacagi buffer
	while (fscanf(fp, "%s", buffer) != EOF){
		int i = 0;
		int key = horner(buffer, 997); // kelimenin hornere göre degeri
		
		// bos goz bulana kadar double hashing ile key degerini degistirme
		while (smallDict[key][0] != NULL){
			i++; // eger mevcut goz doluysa 
			key = horner(buffer, 997) + i*(1 + horner(buffer, 996)); 
			key = key % 997;
			
			// 997 adimda goz bulunamadiysa tablo dolu demektir. Ancak bu odevde bu olmayacagi icin detaylandirilip bu olasilik incelenmedi.
			if (i >= 997){
				break;
			}
		}
		// bos goz bulununca o goze kelimenin yazilmasi
		strcpy(smallDict[key], buffer);
	}
	fclose(fp);
}

// yanlis bir kelime ve dogrusunu hatali kelimelerin bulundugu ikinci hashtable'a ekleyen fonksiyon
void loadWrongTable(char *wrong, char *correct){
	int i = 0;
	int key = horner(wrong, 997);
	
	// bos goz bulana kadar gidecek
	while (wrongTable[key].wrong != NULL){
		i++; // eger mevcut goz doluysa 
		key = horner(wrong, 997) + i*(1 + horner(wrong, 996)); 
		key = key % 997;
		if (i >= 997){
			break;
		}
	}
	
	// bos goz bulunca hatali kelimeyi ve dogrusu yerlestirildi
	strcpy(wrongTable[key].wrong, wrong);
	strcpy(wrongTable[key].correct, correct);
}

// verilen iki kelime icin edit distance algoritmasina gore distance hesabi yapan fonksiyon
// K = 2 degeri baz alinarak 2 den buyuk degerler INT_MAX olarak donulur
int editDistance(char *word1, char *word2){
	
	int K = 2;
	int len1 = strlen(word1);
	int len2 = strlen(word2);
	
	// bonus icin bir ekstra yaklasim. eger iki kelimenin uzunluk farký 2 den buyukse 
	// bu kelimelerin distancelari kesinlikle 2^den buyuktur.
	if (abs(len1-len2)>K){
		return INT_MAX;
	}
	
	int dparray[len1+1][len2+1]; // dinamik programlama matrisi
	
	int i;
	int j;
	
	// ilk sutun 0,1,2,...,len1 ile dolduruldu
	for (i = 0; i < len1+1; i++){
		dparray[i][0] = i;
	}
	// ilk satir 0,1,2,...,len2 ile dolduruldu
	for (i = 1; i < len2+1; i++){
		dparray[0][i] = i;
	}
	
	// BONUS ACIKLAMA!!!
	// dinamik programlama ile distance hesabý bu kisimda yapilir.
	// bonus olarak istenen sarti saglamak icin yapilan islem bir minROW degerinin takibidir.
	// buna gore eger bir satirin tamamý K=2'den buyukse direkt olarak INT_MAX return edilir.
	// cunku bir satirin tamamý 2'den buyukken alttaki satirlardaki sonuclar da kesin olarak 2'den buyuk olacaktir.
	// yani distance kesin olarak 2'den buyuk olacaktir.
	// bu nedenle her bir satir gezilirken o satirin minimumu bir degiskende tutulur (minROW)
	// satir bittiginde ise bu minROW K=2 ile kýyaslanýr. Daha buyukse islem yarida kesilir ve fonksiyondan erkenden cikilir.
	// bu sayede tum matrisi hesaplamaya gerek kalmaz.
	
	int minofROW; // satirlarýn minimumunun tutulacagi deger
	for (i = 1; i < len1+1; i++){
		
		minofROW = i; // initial olarak satirin ilk elemani verildi.
		
		for (j = 1; j < len2+1; j++){
			
			// algoritmaya gore mevcut satir ve sutunda harfler ayniysa 
			// deger sol ust caprazdan alinir (array[i][j] = arrray[i-1][j-1])
			if (word1[i-1] == word2[j-1]){
				dparray[i][j] = dparray[i-1][j-1];
			}
			
			// eger harfler farkliysa
			// sol, ust ve solust degerlerden minimum olana 1 degerinde bir cost eklenerek mecvut degere karar verilir.
			// array[i][j] = 1 + min(array[i-1][i-1], array[i-1][j], array[i][j-1])
			else{
				if (dparray[i-1][j-1] <= dparray[i][j-1] && dparray[i-1][j-1] <= dparray[i-1][j]){
					dparray[i][j] = dparray[i-1][j-1] + 1;
				}
				else if (dparray[i][j-1] <= dparray[i-1][j-1] && dparray[i][j-1] <= dparray[i-1][j]){
					dparray[i][j] = dparray[i][j-1] + 1;
				}
				else{
					dparray[i][j] = dparray[i-1][j] + 1;
				}
			}
			// her bir deger girildiginde minofROW ile kiyaslanarak satirin minimumu bulunur.
			if (dparray[i][j] < minofROW){
				minofROW = dparray[i][j];
			}
		}
		
		// satirin minimumu K ile kiyaslanir ve eger K'dan buyukse fonksiyondan erken cikilir.
		if (minofROW > K){
			return INT_MAX;
		}
	}
	
	// programdan erken cikilamama durumuna gore eger en sag alt deger K'dan buyukse INT_MAX, degilse K degeri donulur.
	if (dparray[len1][len2] <= 2){
		return dparray[len1][len2];
	}
	else{
		return INT_MAX;
	}
}


// bir kelime hem sozlukte hem de hatali kelime tablosunda yoksa kelimeler oneren fonksiyon
// pos da test kelimesinin cumleKelimeler listesindeki konumudur
void suggested (char *word, int pos){
	int i;
	int distanceList[997]; // kelimenin tum sozluk kelimelerine uzakligini tutan dizi
	
	// bir kelimenin sozlukteki i. siradaki kelimeye uzakligi distanceList[i]'de saklanir.
	for (i = 0; i < 997; i++){
		// eger tablonun o gozu bassa INT_MAX yazilir
		if (smallDict[i][0] == NULL){
			distanceList[i] = INT_MAX;
		}
		
		// doluysa editDistance fonksiyonu cagrilarak mesafesi yazilir.
		else{
			distanceList[i] = editDistance(word, smallDict[i]);
		}
	}
	
	// minimum uzaklýktaki kelimenin ne kadar uzaklikta oldugu bulunur
	int min = distanceList[0];
	for (i = 1; i < 997; i++){
		if (distanceList[i] < min){
			min = distanceList[i];
		}
	}
	// eger minimum uzaklýk INT_MAX ise kelimeye 2 veya daha az uzaklikta hic bir kelime yoktur
	if (min == INT_MAX){
		printf("%s is not found!! Word will not be changed!!!\n", cumleKelimeler[pos]);
	}
	// eger minimum uzaklýk INT_MAX degilse 2 vceya 1'dir. Buna gore bu deger kac ise o deger uzakliktaki kelimeler kullaniciya onerilir.
	else{
		printf("%s is not in the dictionary. Do you mean: ", cumleKelimeler[pos]);
		onerilenindex = 0;
		for (i = 0; i < 997; i++){
			if (distanceList[i] == min){
				strcpy(onerilenKelimeler[onerilenindex], smallDict[i]); // onerilen kelimeler kontrol icin bir listede tutulur.
				printf("%s  ", smallDict[i]);
				onerilenindex++;
			}
		}
		
		// kullanicidan kelimenin dogrusu alinir
		
		int dogruKelime = 0;
		char correctW[20];
		
		// kullanicidan alinan kelime, onerilen kelime listesinde var mý diye kontrol edilir.
		// listede yoksa hata verilir ve tekrar sorulur
		// varsa dogru kelime olarak kadedilir.
		while (dogruKelime == 0){
			printf("\nWord: ");
			scanf("%s", correctW);
			getchar();
			
			for (i = 0; i < onerilenindex; i++){
				if (strcmp(correctW, onerilenKelimeler[i]) == 0){
					dogruKelime = 1;
				}
			}
			if (dogruKelime == 0){
				printf("Hatali Kelime Girdiniz Tekrar Deneyin!! ");
			}
		}
				
		// hatali kelime ve dogrusu hatali kelime tablosuna eklenir
		loadWrongTable(word, correctW);
		
		// hatali kelime cumleKelimeler listesinde dogrusutla degistirilir.
		strcpy(cumleKelimeler[pos], correctW);
	}
}


// bir kelimeyi hatali kelime tablosunda arayan fonksiyon
int searchWrongs (char *word, int pos){
	int key = horner(word, 997); // initial anahtar degeri
	int i = 0;
	while (wrongTable[key].wrong != NULL && strcmp(wrongTable[key].wrong, word) != 0){
		// bulunana kadar veya NULL gelene kadar aramaya devam edilecek.
		i++;
		key = horner(word, 997) + i*(1 + horner(word, 996)); 
		key = key % 997;
		if (i >= 997)// 997 adimda bulunamazsa arama durdurulmali.
			break;
	}
	// eger kelime hatali kelimeler tablosunda var ise cumleKelimeler listesinde dogrusu ile degistirilir
	if (strcmp(wrongTable[key].wrong, word) == 0){
		strcpy(cumleKelimeler[pos], wrongTable[key].correct);
	}
	// kelime hatali kelimeler tablosunda yok ise yakin kelimeler onerilir.
	else{
		suggested(word, pos);
	}
}


// bir kelimeyi smallDict sozlugunde arayan fonksiyon
void search(char *word, int pos){
	int len = strlen(word); // kelime uzunluðu
	int i;
	// kelimeler kucuk harfle saklandigi icin kucuk harfe cevrilip aranir
	for (i = 0; i < len; i++){
		word[i] = tolower(word[i]);
	}
	
	int key = horner(word, 997); // initial anahtar degeri
	
	i = 0;
	// bos goz bulana kadar ya da kelime bulunana kadar aranir
	while (smallDict[key][0] != NULL && strcmp(smallDict[key], word) != 0){
		// bulunana kadar veya NULL gelene kadar aramaya devam edilecek.
		i++;
		key = horner(word, 997) + i*(1 + horner(word, 996)); 
		key = key % 997;
		if (i >= 997)// 997 adimda bulunamazsa arama durdurulmali.
			break;
	}
	// kelime bulunamazsa bos goz bulundu demektir. Bu nedenle hatali kelimeler tablosunda aranir.
	// kelime bulunursa hic bir islem yapilmaz
	if (strcmp(smallDict[key], word) != 0){
		searchWrongs(word, pos);
	}
}

int main(void){
	
	loadTable("smallDictionary.txt"); // smallDict hashtable'i olusturulur
	
	char cumle[500]; // cumle burada tutulur
	char *buffer; // cumleyi kelimelere ayirirken buffer kullanilir
	const char del[2] = " "; // strtok kullanilacaktýr. Buna gore kelime ayirmak icin space karakteri kullanilir.
	int i;
	
	// kullanici kapatana kadar islem yapabilir
	while (1){
		printf("\nCumle: ");
		// cumle okundu
		scanf("%[^\n]s", cumle);
		getchar();
		kelimeSayisi = 0; // kelime sayisi 0'a cekildi boylede cumleKelime listesinde 0'dan yerlestirilmeye baslanacak
		
		
		buffer = strtok(cumle, del); // cumlenin ilk kelimesi buffera alindi
		while (buffer != NULL){
			strcpy(cumleKelimeler[kelimeSayisi], buffer); // kelime cumleKelimeler listesine yazildi
			kelimeSayisi++; // sonraki indexe gecildi
			buffer = strtok(NULL, del); // bir sonraki kelime buffera alindi
		}
		
		char buffer2[20]; // kelimeleri search fonksiyonuna gondermek icin buffer kullanilmaktadir.
		// Boylece kelime arama yapilacagi zaman kucuk harfe donusecekken cumleKelimeler listesinde ayni kalmaktadir.
		// kelimenin orjinali boyle korunur
		for (i = 0; i < kelimeSayisi; i++){
			strcpy(buffer2, cumleKelimeler[i]);
			search(buffer2, i);
		}
		
		// cumle kelimeler listesi ekrana yazilir.
		for (i = 0; i < kelimeSayisi; i++){
			printf("%s ", cumleKelimeler[i]);
		}
		printf("\n");
	}
	
	return 0;
}
