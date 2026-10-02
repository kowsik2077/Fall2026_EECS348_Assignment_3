/*
Program Name: EECS 348 Assignment 3 - CEO Email Priority Queue (email_priority.cpp)
Description: C++ object-oriented program that prioritizes emails for a busy CEO.
  It uses a MaxHeap, built from scratch on a dynamically resized array, as a
  priority queue. Emails are ranked by sender category (Boss, Subordinate, Peer,
  ImportantPerson, OtherPerson) and, within a category, the newest email is read first.
  No pre-existing heap modules (std::priority_queue, std::make_heap, etc.) are used.
Inputs: A test file whose path is given as the first command-line argument (if no
  argument is given, commands are read from standard input). Each line is one of:
  EMAIL <sender category>,<subject line>,<date MM-DD-YYYY> | NEXT | READ | COUNT
Output: Terminal output only.
  COUNT prints "There are N emails to read."
  NEXT prints "Next email:" followed by the Sender, Subject, and Date lines.
  READ prints nothing (it removes the highest-priority email).
Collaborators: ChatGPT (OpenAI) and Gemini (Google) generated the two programs I
  compared in my GenAI analysis.
Other Sources: The baseline code is ChatGPT's. Gemini's program was used only for
  comparison (I borrowed its idea of stripping carriage returns)
Author: Bikramaditya Kowsik
Creation Date: October 1, 2026
Revision Date: October 1, 2026
Revisions: Improved the ChatGPT baseline: trim carriage returns and whitespace from
  input, parse rank and date once per email, add an arrival-order tie-breaker, move
  elements with a "hole" instead of swapping, forbid copying of MaxHeap, remove
  "using namespace std", name the rank constants, validate input so it cannot crash,
  read standard input when no file is given, and use "\n" instead of std::endl.
Authorship key: The code is ChatGPT's baseline. Lines I wrote or changed are
  marked [My work]; every unmarked line is ChatGPT's code.
*/

#include <iostream> // provides std::cout, std::cerr, and std::cin
#include <fstream> // provides std::ifstream for reading the test file
#include <string> // provides std::string
#include <cstddef> // [My work] provides std::size_t

/*
Email class
Stores one email and knows how to compare its priority against another email.
The category rank and numeric date are computed once, when the object is created.
*/
class Email { // begin the Email class
public: // public section holds the named constants and the interface
    enum Rank { // [My work] named constants replace the magic numbers from the baseline
        RANK_UNKNOWN = 0, // [My work] rank for a sender string that is not one of the five categories
        RANK_OTHER_PERSON = 1, // [My work] lowest real priority, read last
        RANK_IMPORTANT_PERSON = 2, // [My work] read after Peer
        RANK_PEER = 3, // [My work] read after Subordinate
        RANK_SUBORDINATE = 4, // [My work] read after Boss
        RANK_BOSS = 5 // [My work] highest priority, read first
    }; // end of the Rank enumeration

private: // data members and helpers are hidden from other classes
    std::string sender; // sender category exactly as it appeared in the file
    std::string subject; // subject line of the email
    std::string date; // date text in MM-DD-YYYY format, printed back unchanged
    int categoryRank; // [My work] cached numeric rank of the sender category
    int dateValue; // [My work] cached date as YYYYMMDD (fits in an int) so dates compare as plain numbers
    unsigned long sequence; // [My work] arrival order of the email, used only to break exact ties

    // Converts a sender category string into its numeric rank
    static int rankFor(const std::string& s) { // [My work] static because it needs no Email object
        if (s == "Boss") return RANK_BOSS; // [My work] Boss uses a named constant and is read first
        if (s == "Subordinate") return RANK_SUBORDINATE; // [My work] Subordinate uses a named constant and is read next
        if (s == "Peer") return RANK_PEER; // [My work] Peer uses a named constant and is read next
        if (s == "ImportantPerson") return RANK_IMPORTANT_PERSON; // [My work] ImportantPerson uses a named constant and is read next
        if (s == "OtherPerson") return RANK_OTHER_PERSON; // [My work] OtherPerson is read last
        return RANK_UNKNOWN; // [My work] unrecognized senders go behind everyone instead of being treated as OtherPerson
    } // end of rankFor

    // Converts MM-DD-YYYY into the number YYYYMMDD, or 0 if the text is malformed
    static int dateToNumber(const std::string& d) { // [My work] replaces the stringstream parsing
        if (d.length() != 10) return 0; // [My work] a valid date has exactly 10 characters
        if (d[2] != '-' || d[5] != '-') return 0; // [My work] the two separators must be dashes
        const int digitPositions[8] = {0, 1, 3, 4, 6, 7, 8, 9}; // [My work] indexes that must hold digits
        for (int i = 0; i < 8; ++i) { // [My work] check each digit position in turn
            char c = d[digitPositions[i]]; // [My work] character at this position
            if (c < '0' || c > '9') return 0; // [My work] reject non-digits so no exception can occur
        } // end of digit check loop
        int month = (d[0] - '0') * 10 + (d[1] - '0'); // [My work] month from the first two digits
        int day = (d[3] - '0') * 10 + (d[4] - '0'); // [My work] day from digits 4 and 5
        int year = (d[6] - '0') * 1000 + (d[7] - '0') * 100 + (d[8] - '0') * 10 + (d[9] - '0'); // [My work] four-digit year
        return year * 10000 + month * 100 + day; // [My work] combine into YYYYMMDD (max 99991231, so an int is enough)
    } // end of dateToNumber

public: // public interface of the Email class
    Email() : sender(""), subject(""), date(""), categoryRank(RANK_UNKNOWN), dateValue(0), sequence(0) {} // [My work] default constructor required by new Email[n]

    Email(const std::string& s, const std::string& sub, const std::string& d, unsigned long seq) // [My work] main constructor, now also takes the arrival number
        : sender(s), subject(sub), date(d), categoryRank(rankFor(s)), dateValue(dateToNumber(d)), sequence(seq) {} // [My work] rank and date are computed once here

    std::string getSender() const { return sender; } // returns the sender category
    std::string getSubject() const { return subject; } // returns the subject line
    std::string getDate() const { return date; } // returns the date text

    // Returns true if this email must be read before the other email
    bool higherPriorityThan(const Email& other) const { // [My work] the single comparison used by the heap
        if (categoryRank != other.categoryRank) return categoryRank > other.categoryRank; // higher sender category wins
        if (dateValue != other.dateValue) return dateValue > other.dateValue; // [My work] same category: newer date wins, compared as cached integers
        return sequence < other.sequence; // [My work] same category and date: the email that arrived first wins
    } // end of higherPriorityThan
}; // end of the Email class

/*
MaxHeap class
List-based (dynamic array) binary max-heap of Email objects, built from scratch.
The highest-priority email is always stored at index 0.
*/
class MaxHeap { // begin the MaxHeap class
private: // internal state and helpers
    Email* items; // [My work] dynamically allocated array that stores the heap
    std::size_t count; // [My work] number of emails currently stored
    std::size_t capacity; // [My work] number of slots allocated in the array

    // Doubles the array size when it is full
    void grow() { // [My work] amortized O(1) per insert because capacity doubles
        std::size_t newCapacity = capacity * 2; // new array is twice as large
        Email* bigger = new Email[newCapacity]; // allocate the larger array
        for (std::size_t i = 0; i < count; ++i) { // copy every stored email
            bigger[i] = items[i]; // copy one email into the new array
        } // end of copy loop
        delete[] items; // free the old array so no memory leaks
        items = bigger; // switch to the new array
        capacity = newCapacity; // remember the new size
    } // end of grow

    // Moves the email at index up until its parent has higher priority
    void siftUp(std::size_t index) { // [My work] uses a hole instead of repeated swaps
        Email moving = items[index]; // [My work] hold the email being placed (one copy instead of one per swap)
        while (index > 0) { // stop at the root
            std::size_t parent = (index - 1) / 2; // parent index in an array-based heap
            if (!moving.higherPriorityThan(items[parent])) break; // [My work] stop when the parent already outranks it
            items[index] = items[parent]; // [My work] slide the parent down into the hole
            index = parent; // the hole moves up one level
        } // end of sift-up loop
        items[index] = moving; // [My work] drop the held email into its final position
    } // end of siftUp

    // Moves the email at index down until both children have lower priority
    void siftDown(std::size_t index) { // [My work] uses a hole instead of repeated swaps
        Email moving = items[index]; // [My work] hold the email being placed
        while (true) { // loop until it reaches its place
            std::size_t child = 2 * index + 1; // start with the left child
            if (child >= count) break; // [My work] no children means it is already a leaf
            if (child + 1 < count && items[child + 1].higherPriorityThan(items[child])) { // [My work] pick the higher-priority child
                ++child; // [My work] the right child outranks the left child
            } // end of child selection
            if (!items[child].higherPriorityThan(moving)) break; // [My work] stop when the best child does not outrank it
            items[index] = items[child]; // [My work] slide the better child up into the hole
            index = child; // the hole moves down one level
        } // end of sift-down loop
        items[index] = moving; // [My work] drop the held email into its final position
    } // end of siftDown

public: // public interface of the MaxHeap class
    MaxHeap() : items(new Email[16]), count(0), capacity(16) {} // [My work] start with room for 16 emails

    ~MaxHeap() { delete[] items; } // release the array when the heap is destroyed

    MaxHeap(const MaxHeap&) = delete; // [My work] forbid copying so two heaps can never delete[] the same array
    MaxHeap& operator=(const MaxHeap&) = delete; // [My work] forbid assignment for the same reason

    void insert(const Email& email) { // add an email to the heap in O(log n)
        if (count == capacity) grow(); // make room if the array is full
        items[count] = email; // place the new email at the end
        ++count; // one more email is stored
        siftUp(count - 1); // restore the heap order
    } // end of insert

    bool isEmpty() const { return count == 0; } // true when no emails are waiting

    std::size_t getSize() const { return count; } // number of unread emails in O(1)

    const Email* getMax() const { // look at the top email without removing it
        if (isEmpty()) return nullptr; // nothing to return when empty
        return &items[0]; // the root is always the highest-priority email
    } // end of getMax

    void removeMax() { // [My work] delete the top email in O(log n)
        if (isEmpty()) return; // READ on an empty queue does nothing
        --count; // [My work] shrink the logical size first
        if (count > 0) { // only rearrange when emails remain
            items[0] = items[count]; // [My work] move the last email to the root
            siftDown(0); // push it down to its correct place
        } // end of rearrange step
    } // end of removeMax
}; // end of the MaxHeap class

/*
EmailManager class
Reads commands, cleans up each line, and sends the work to the MaxHeap.
*/
class EmailManager { // begin the EmailManager class
private: // internal state and helpers
    MaxHeap emailQueue; // the priority queue of unread emails
    unsigned long nextSequence; // [My work] counter that numbers emails in arrival order

    // Removes leading and trailing spaces, tabs, and carriage returns [My work]
    static std::string trim(const std::string& text) { // [My work] needed because Windows files end lines with \r
        const char* whitespace = " \t\r\n"; // [My work] characters treated as whitespace
        std::size_t first = text.find_first_not_of(whitespace); // [My work] index of first real character
        if (first == std::string::npos) return ""; // [My work] a blank line becomes an empty string
        std::size_t last = text.find_last_not_of(whitespace); // [My work] index of last real character
        return text.substr(first, last - first + 1); // [My work] return only the middle part
    } // end of trim

    // Handles one "EMAIL <category>,<subject>,<date>" line
    void processEmail(const std::string& line) { // [My work] parse and insert one email
        std::string data = line.substr(6); // drop the leading "EMAIL "
        std::size_t firstComma = data.find(','); // comma that ends the sender field
        std::size_t lastComma = data.rfind(','); // [My work] comma that starts the date field
        if (firstComma == std::string::npos || firstComma == lastComma) return; // [My work] ignore lines with fewer than two commas
        std::string sender = trim(data.substr(0, firstComma)); // [My work] sender category without stray spaces
        std::string subject = data.substr(firstComma + 1, lastComma - firstComma - 1); // subject is everything between the commas
        std::string date = trim(data.substr(lastComma + 1)); // [My work] date without stray spaces
        emailQueue.insert(Email(sender, subject, date, nextSequence)); // [My work] build the Email and queue it
        ++nextSequence; // [My work] the next email gets the next arrival number
    } // end of processEmail

    void processNext() const { // [My work] show the next email without removing it
        const Email* email = emailQueue.getMax(); // peek at the highest-priority email
        if (email == nullptr) return; // print nothing when the queue is empty
        std::cout << "Next email:" << "\n"; // [My work] heading line; "\n" replaces std::endl so the stream is not flushed on every line
        std::cout << "Sender: " << email->getSender() << "\n"; // [My work] sender line, no forced flush
        std::cout << "Subject: " << email->getSubject() << "\n"; // [My work] subject line, no forced flush
        std::cout << "Date: " << email->getDate() << "\n"; // [My work] date line, no forced flush
    } // end of processNext

    void processRead() { emailQueue.removeMax(); } // the CEO dealt with the top email, so remove it

    void processCount() const { // [My work] report how many emails are unread
        std::cout << "There are " << emailQueue.getSize() << " emails to read." << "\n"; // [My work] count message from the assignment, no forced flush
    } // end of processCount

public: // public interface of the EmailManager class
    EmailManager() : nextSequence(0) {} // [My work] arrival numbering starts at zero

    void processCommand(const std::string& rawLine) { // [My work] dispatch one command line
        std::string command = trim(rawLine); // [My work] clean the line before matching it
        if (command.compare(0, 6, "EMAIL ") == 0) { // line starts with "EMAIL "
            processEmail(command); // add the email
        } else if (command == "NEXT") { // NEXT command
            processNext(); // display the top email
        } else if (command == "READ") { // READ command
            processRead(); // remove the top email
        } else if (command == "COUNT") { // COUNT command
            processCount(); // display the unread count
        } // any other line (including blank lines) is ignored
    } // end of processCommand

    void processStream(std::istream& input) { // [My work] works for both a file stream and standard input
        std::string line; // holds one line of input
        while (std::getline(input, line)) { // read until the end of the input
            processCommand(line); // run each line as a command
        } // end of read loop
    } // end of processStream
}; // end of the EmailManager class

int main(int argc, char* argv[]) { // [My work] program entry point, the only code outside a class
    EmailManager manager; // object that runs the whole program
    if (argc >= 2) { // [My work] a test file name was given
        std::ifstream input(argv[1]); // open the test file
        if (!input) { // [My work] file could not be opened
            std::cerr << "Error: cannot open file " << argv[1] << std::endl; // [My work] tell the user why nothing happened
            return 1; // exit with a failure code
        } // end of open check
        manager.processStream(input); // [My work] process every command in the file
    } else { // [My work] no file name was given
        manager.processStream(std::cin); // [My work] read commands from standard input instead
    } // end of input selection
    return 0; // exit successfully
} // end of main