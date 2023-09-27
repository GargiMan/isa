# isa

Napište program dns, který bude umět zasílat dotazy na DNS servery a v čitelné podobě vypisovat přijaté odpovědi na standardní výstup. Sestavení a analýza DNS paketů musí být implementována přímo v programu dns. Stačí uvažovat pouze komunikaci pomocí UDP.

Není-li v jiné části zadání specifikováno jinak, je při vytváření programu povoleno použít hlavičkové soubory pro práci se sokety a další obvyklé funkce používané v síťovém prostředí (jako je netinet/*, sys/*, arpa/* apod.), knihovnu pro práci s vlákny (pthread), signály, časem, stejně jako standardní knihovnu jazyka C (varianty ISO/ANSI i POSIX), C++ a STL. Jiné knihovny nejsou povoleny.

### Spuštění aplikace  
Použití: ```dns [-r] [-x] [-6] -s server [-p port] adresa```

Pořadí parametrů je libovolné. Popis parametrů:

| Parametr | Popis                                                         |
| -------- | ------------------------------------------------------------- |
| -r       | Požadována rekurze (Recursion Desired = 1), jinak bez rekurze |
| -x       | Reverzní dotaz místo přímého                                  |
| -6       | Dotaz typu AAAA místo výchozího A                             |
| -s       | IP adresa nebo doménové jméno serveru, kam se má zaslat dotaz |
| -p port  | Číslo portu, na který se má poslat dotaz, výchozí 53          |
| adresa   | Dotazovaná adresa                                             |

### Podporované typy dotazů

Program dns se v odpovědích musí vypořádat se záznamy typu CNAME. Není požadována podpora DNSSEC.

### Výstup aplikace

Na standardní výstup vypište informaci o tom, zda je získaná odpověď autoritativní, zda byla zjištěna rekurzivně a zda byla odpověď zkrácena. Dále vypište všechny sekce a záznamy obdržené v odpovědi.

Pro každou sekci v odpovědi vypište její název a počet získaných záznamů. Pro každý záznam vypište jeho název, typ, třídu, TTL a data.

#### Ukázka možného výstupu:

```
$ dns -r -s kazi.fit.vutbr.cz www.fit.vut.cz
Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  www.fit.vut.cz., A, IN
Answer section (1)
  www.fit.vut.cz., A, IN, 14400, 147.229.9.26
Authority section (0)
Additional section (0)

$ dns -r -s kazi.fit.vutbr.cz www.github.com
Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  www.github.com., A, IN
Answer section (2)
  www.github.com., CNAME, IN, 3600, github.com.
  github.com., A, IN, 60, 140.82.121.3
Authority section (0)
Additional section (0)
```

### Doplňující informace k zadání

- Před odevzdáním projektu si důkladně pročtěte společné zadání pro všechny projekty.
- Jakékoliv rozšíření nezapomeňte zdůraznit v souboru README a v dokumentaci. Není však možné získat více bodů, než je stanovené maximum.
- Program se musí vypořádat s chybnými vstupy.
- Veškeré chybové výpisy vypisujte srozumitelně na standardní chybový výstup.
- Pokud máte pocit, že v zadání není něco specifikováno, popište v dokumentaci vámi zvolené řešení a zdůvodněte, proč jste jej vybrali.
- Vytvořený kód by měl být modulární a otestovaný. Testy, které jste při řešení projektu napsali se spustí voláním `make test`.
- Pište robustní aplikace, které budou vstřícné k drobným odchylkám od specifikace.
- Při řešení projektu uplatněte znalosti získané v dřívějších kurzech týkající se jak zdrojového kódu (formátování, komentáře), pojmenování souborů, tak vstřícnosti programu k uživateli.

### Referenční prostředí pro překlad a testování
Program by měl být přenositelný. Referenční prostředí pro překlad budou servery eva.fit.vutbr.cz a merlin.fit.vutbr.cz (program musí být přeložitelný a funkční na obou systémech). Vlastní testování může probíhat na jiném počítači s nainstalovaným OS GNU/Linux, či FreeBSD, včetně jiných architektur než Intel/AMD, jiných distribucí, jiných verzí knihoven apod. Pokud vyžadujete minimální verzi knihovny (dostupné na serveru merlin a eva), jasně tuto skutečnost označte v dokumentaci a README.

### Doporučená literatura

[RFC1035](https://tools.ietf.org/html/rfc1035)  
[RFC3596](https://tools.ietf.org/html/rfc3596)
