
#include "stdafx.h"
#include "wordmgr.h"

static const std::string stopwords[] = {
    "!!", "?!", "??", "!?", "`", "``", "''", "-lrb-", "-rrb-", "-lsb-", "-rsb-", ",", ".", ":", ";", "'", "?", "<", ">", "{", "}",
    "[", "]", "+", "-", "(", ")", "&", "%", "$", "@", "!", "^", "#", "*", "..", "...", "'ll", "'s", "'m", "a", "about", "above",
    "after", "again", "against", "all", "am", "an", "and", "any", "are", "aren't", "as", "at", "be", "because", "been", "before",
    "being", "below", "between", "both", "but", "by", "can", "can't", "cannot", "could", "couldn't", "did", "didn't", "do", "does",
    "doesn't", "doing", "don't", "down", "during", "each", "few", "for", "from", "further", "had", "hadn't", "has", "hasn't", "have",
    "haven't", "having", "he", "he'd", "he'll", "he's", "her", "here", "here's", "hers", "herself", "him", "himself", "his", "how",
    "how's", "i", "i'd", "i'll", "i'm", "i've", "if", "in", "into", "is", "isn't", "it", "it's", "its", "itself", "let's", "me",
    "more", "most", "mustn't", "my", "myself", "no", "nor", "not", "of", "off", "on", "once", "only", "or", "other", "ought", "our",
    "ours ", "ourselves", "out", "over", "own", "same", "shan't", "she", "she'd", "she'll", "she's", "should", "shouldn't", "so",
    "some", "such", "than", "that", "that's", "the", "their", "theirs", "them", "themselves", "then", "there", "there's", "these",
    "they", "they'd", "they'll", "they're", "they've", "this", "those", "through", "to", "too", "under", "until", "up", "very",
    "was", "wasn't", "we", "we'd", "we'll", "we're", "we've", "were", "weren't", "what", "what's", "when", "when's", "where",
    "where's", "which", "while", "who", "who's", "whom", "why", "why's", "with", "won't", "would", "wouldn't", "you", "you'd",
    "you'll", "you're", "you've", "your", "yours", "yourself", "yourselves", "###", "return", "arent", "cant", "couldnt", "didnt",
    "doesnt", "dont", "hadnt", "hasnt", "havent", "hes", "heres", "hows", "im", "isnt", "its", "lets", "mustnt", "shant", "shes",
    "shouldnt", "thats", "theres", "theyll", "theyre", "theyve", "wasnt", "were", "werent", "whats", "whens", "wheres", "whos",
    "whys", "wont", "wouldnt", "youd", "youll", "youre", "youve",
    "a","as","able","about","above","according","accordingly","across","actually","after","afterwards","again","against","aint",
    "all","allow","allows","almost","alone","along","already","also","although","always","am","among","amongst","an","and","another","any",
    "anybody","anyhow","anyone","anything","anyway","anyways","anywhere","apart","appear","appreciate","appropriate","are","arent","around",
    "as","aside","ask","asking","associated","at","available","away","awfully","b","be","became","because","become","becomes","becoming",
    "been","before","beforehand","behind","being","believe","below","beside","besides","best","better","between","beyond","both","brief",
    "but","by","c","cmon","cs","came","can","cant","cannot","cant","cause","causes","certain","certainly","changes","clearly","co","com",
    "come","comes","concerning","consequently","consider","considering","contain","containing","contains","corresponding","could","couldnt",
    "course","currently","d","definitely","described","despite","did","didnt","different","do","does","doesnt","doing","dont","done","down",
    "downwards","during","e","each","edu","eg","eight","either","else","elsewhere","enough","entirely","especially","et","etc","even","ever",
    "every","everybody","everyone","everything","everywhere","ex","exactly","example","except","f","far","few","fifth","first","five","followed",
    "following","follows","for","former","formerly","forth","four","from","further","furthermore","g","get","gets","getting","given","gives",
    "go","goes","going","gone","got","gotten","greetings","h","had","hadnt","happens","hardly","has","hasnt","have","havent","having","he",
    "hes","hello","help","hence","her","here","heres","hereafter","hereby","herein","hereupon","hers","herself","hi","him","himself","his",
    "hither","hopefully","how","howbeit","however","i","id","ill","im","ive","ie","if","ignored","immediate","in","inasmuch","inc","indeed",
    "indicate","indicated","indicates","inner","insofar","instead","into","inward","is","isnt","it","itd","itll","its","its","itself","j",
    "just","k","keep","keeps","kept","know","known","knows","l","last","lately","later","latter","latterly","least","less","lest","let","lets",
    "like","liked","likely","little","look","looking","looks","ltd","m","mainly","many","may","maybe","me","mean","meanwhile","merely","might",
    "more","moreover","most","mostly","much","must","my","myself","n","name","namely","nd","near","nearly","necessary","need","needs","neither",
    "never","nevertheless","new","next","nine","no","nobody","non","none","noone","nor","normally","not","nothing","novel","now","nowhere","o",
    "obviously","of","off","often","oh","ok","okay","old","on","once","one","ones","only","onto","or","other","others","otherwise","ought","our",
    "ours","ourselves","out","outside","over","overall","own","p","particular","particularly","per","perhaps","placed","please","plus","possible",
    "presumably","probably","provides","q","que","quite","qv","r","rather","rd","re","really","reasonably","regarding","regardless","regards",
    "relatively","respectively","right","s","said","same","saw","say","saying","says","second","secondly","see","seeing","seem","seemed","seeming",
    "seems","seen","self","selves","sensible","sent","serious","seriously","seven","several","shall","she","should","shouldnt","since","six","so",
    "some","somebody","somehow","someone","something","sometime","sometimes","somewhat","somewhere","soon","sorry","specified","specify","specifying",
    "still","sub","such","sup","sure","t","ts","take","taken","tell","tends","th","than","thank","thanks","thanx","that","thats","thats","the",
    "their","theirs","them","themselves","then","thence","there","theres","thereafter","thereby","therefore","therein","theres","thereupon",
    "these","they","theyd","theyll","theyre","theyve","think","third","this","thorough","thoroughly","those","though","three","through",
    "throughout","thru","thus","to","together","too","took","toward","towards","tried","tries","truly","try","trying","twice","two","u","un",
    "under","unfortunately","unless","unlikely","until","unto","up","upon","us","use","used","useful","uses","using","usually","uucp","v",
    "value","various","very","via","viz","vs","w","want","wants","was","wasnt","way","we","wed","well","were","weve","welcome","well",
    "went","were","werent","what","whats","whatever","when","whence","whenever","where","wheres","whereafter","whereas","whereby","wherein",
    "whereupon","wherever","whether","which","while","whither","who","whos","whoever","whole","whom","whose","why","will","willing","wish",
    "with","within","without","wont","wonder","would","wouldnt","x","y","yes","yet","you","youd","youll","youre","youve","your","yours",
    "yourself","yourselves","youll","z","zero",
    "s", "t", "don", "aren", "isn", "wasn", "weren", "shouldn", "wouldn", "mustn", "hadn", "couldn", "'t", "'s", "no.", "'re", "ain"
};

wordmgr::wordmgr() {
    _next_idx = 0;
    _frozen = false;
    for (long long i = 0; i < sizeof(stopwords) / sizeof(stopwords[0]); i++) {
        _stopwords.insert(stopwords[i]);
    }
}
wordmgr::~wordmgr() {
}

void wordmgr::append(const char *word) {
#ifdef _DEBUG
    if (this->is_frozen())
        throw std::runtime_error(u8"wordmgr: append: object is frozen");
#endif
    std::string str(word);

    // has alphabet?
    int alphabet_cnt = 0;
    for (int i = 0; i < str.size(); i++) {
        char c = word[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            alphabet_cnt++;
        }
    }
    if (alphabet_cnt <= 1)
        return; // discard special-character-only words
    /*
    if (str.size() <= 1) {
        return; // discard single-character words
    }
    if (str[0] < 'a' || str[0] > 'z') {
        return; // discard things like 've, '90, 3rd, etc
    }
    if (_stopwords.find(str) != _stopwords.end()) {
        return; // discard stopwords
    }
    */
    auto it = _src_dict.find(str);
    if (it == _src_dict.end()) {
        int word_idx = _next_idx++;
        _src_words.push_back(word_idx);
        _src_word_freq.push_back(1);
        _src_word_dict.push_back(str);
        _src_dict.insert(std::make_pair(str, word_idx));
    }
    else {
        int word_idx = it->second;
        _src_words.push_back(word_idx);
        _src_word_freq[word_idx]++;
    }
}
void wordmgr::drop_infrequent_words() {
#ifdef _DEBUG
    if (this->is_frozen())
        throw std::runtime_error(u8"wordmgr: append: object is frozen");
#endif

    std::vector<int> new_indices;
    int count = 0;
    for (int i = 0; i < _src_word_freq.size(); i++) {
        int new_idx = -1;
        if (_src_word_freq[i] >= 5) {
            new_idx = count++;
            _dst_word_dict.push_back(_src_word_dict[i]);
        }
        new_indices.push_back(new_idx);
    }
    for (int i = 0; i < _src_words.size(); i++) {
        int old_idx = _src_words[i];
        int new_idx = new_indices[old_idx];
        if (new_idx != -1) {
            _dst_words.push_back(new_idx);
        }
    }
}

void wordmgr::freeze() {
    drop_infrequent_words();
    _frozen = true;
}

bool wordmgr::is_frozen() const {
    return _frozen;
}

long long wordmgr::word_count() {
    return _src_dict.size();
}

long long wordmgr::corpus_word_count() {
    return _src_words.size();
}

void wordmgr::train_gen_init(int neighbors) {
#ifdef _DEBUG
    if (!this->is_frozen())
        throw std::runtime_error(u8"wordmgr: train_gen_init: object is not frozen");
#endif
    _state.neighbors = neighbors;
    _state.next_center = 0;
    _state.next_offset = -neighbors;
}

long long wordmgr::train_gen(long long desired_count, word_io_pair *buf) {
#ifdef _DEBUG
    if (!this->is_frozen())
        throw std::runtime_error(u8"wordmgr: train_gen: object is not frozen");
#endif
    long long count = 0;
    while (count < desired_count) {
        // try adding current example
        int target_word = _state.next_center + _state.next_offset;
        if (target_word >= 0 && target_word < corpus_word_count()) {
            *buf++ = { _src_words[_state.next_center], _src_words[target_word] };
            count++;
        }
        // advance to next example
        _state.next_offset++;
        if (!_state.next_offset) {
            _state.next_offset++;
        }
        else if (_state.next_offset > _state.neighbors) {
            _state.next_center++;
            if (_state.next_center >= corpus_word_count()) {
                break;
            }
            _state.next_offset = -(_state.neighbors);
        }
    }

    return count;
}

long long wordmgr::train_gen_all(word_io_pair *buf) {
    _state_type _state_backup = _state;
    train_gen_init(_state.neighbors);
    long long gen_count = train_gen(1LL << 48, buf);
    _state = _state_backup;
    return gen_count;
}

int wordmgr::word_to_index(const char *word) const {
#ifdef _DEBUG
    if (!this->is_frozen())
        throw std::runtime_error(u8"wordmgr: word_to_index: object is not frozen");
#endif
    std::string str(word);
    auto it = _src_dict.find(str);
    if (it == _src_dict.end()) {
        return -1;
    }
    else {
        int word_idx = it->second;
        return word_idx;
    }
}

std::string wordmgr::index_to_word(int idx) const {
    if (idx >= 0 && idx < _src_word_dict.size()) {
        return _src_word_dict[idx];
    }
    throw std::runtime_error(u8"wordmgr: index_to_word: idx out of range");
}

