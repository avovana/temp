#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cstdio>
#include <thread>
#include <utility>
#include <unordered_map>
#include <chrono>
#include <list>
#include <future>

using namespace std;

tuple<int, string, tm, tm> get_start_end_times(const string & time_range) {
    int error_code = 0;
    string error_msg = "Time parse failed.";

    char delimiter = '/';
    size_t pos = time_range.find(delimiter);

    if(pos == string::npos) {
        return {1, error_msg, {}, {}};
    }

    string start_time;
    string end_time;

    start_time = time_range.substr(0, pos);
    end_time = time_range.substr(pos + 1, time_range.size() - 1);

    if(start_time.empty() || end_time.empty()) {
        return {2, error_msg, {}, {}};
    }

    tm start_time_t = {};
    tm end_time_t = {};

    istringstream ss;
    ss.str(start_time);

    ss >> get_time(&start_time_t, "%Y%m%d-%H:%M:%S");
    if (ss.fail())
        return {3, error_msg, {}, {}};

    ss.str(end_time);

    ss >> get_time(&end_time_t, "%Y%m%d-%H:%M:%S");
    if (ss.fail())
        return {4, error_msg, {}, {}};

    return {error_code, {}, start_time_t, end_time_t};
}

tuple<int, string> get_tag_value(const string& tag_value) {
    auto delimiter_pos = tag_value.find("=");
    int tag = stoi(tag_value.substr(0, delimiter_pos));
    string value = tag_value.substr(delimiter_pos + 1, tag_value.size() - 1);

    return {tag, value};
}

void filter_messages(promise<list<string>> && p, const string & file_content, size_t msg_start_pos, size_t end_pos, tm start_t, tm end_t, const unordered_map<int, string>& tags_to_check) {
    // cout << __PRETTY_FUNCTION__ << endl;
    // cout << "start_msg_pos: " << msg_start_pos << endl;
    // cout << "end_pos:       " << end_pos << endl << endl;
    list<string> messages;

    char soh = 0x01;

    istringstream ss;
    tm time_value_t = {};
    for(size_t i = msg_start_pos; i < end_pos;) {
        auto time_start_pos = file_content.find("52=", i) + 3;
        auto time_end_pos = file_content.find(".", time_start_pos);
        string time_value = file_content.substr(time_start_pos, time_end_pos - time_start_pos);

        // cout << "time_start_pos: " << time_start_pos << endl;
        // cout << "time_end_pos:   " << time_end_pos << endl;
        // cout << "time_value:     " << time_value << endl;

        ss.clear();
        ss.str(time_value);
        ss >> get_time(&time_value_t, "%Y%m%d-%H:%M:%S");
        if (ss.fail()) {
            p.set_exception(make_exception_ptr(runtime_error("Message invalid time format to parse: " + time_value)));
            return;
        }

        auto msg_time = chrono::system_clock::from_time_t(mktime(&time_value_t));
        auto start_time = chrono::system_clock::from_time_t(mktime(&start_t));
        auto end_time = chrono::system_clock::from_time_t(mktime(&end_t));

        auto new_msg_start_pos = file_content.find("8=FIX.4", time_end_pos);
        if(new_msg_start_pos == string::npos)
            new_msg_start_pos = file_content.size() - 1;

        // cout << "New message -------------------------------------------------------" << endl;
        if(start_time <= msg_time && msg_time <= end_time) {
            bool all_tags_equal = true;
            if(not tags_to_check.empty())
                for(size_t j = i; j < new_msg_start_pos;) {
                    auto tag_end_pos = file_content.find(soh, j);
                    string tag_value = file_content.substr(j, tag_end_pos - j);
                    const auto& [tag, value] = get_tag_value(tag_value);

                    if(tags_to_check.find(tag) != tags_to_check.end())
                        if(tags_to_check.at(tag) != value) {
                            all_tags_equal = false;
                            break;
                        }

                    // cout << tag_value << " " << tag << " " << value << endl;
                    j = tag_end_pos + 1;
                }

            if(all_tags_equal) {
                auto message = file_content.substr(msg_start_pos, new_msg_start_pos - msg_start_pos);
                messages.push_back(message);
                // cout << "msg added" << endl;
            }
        }
        // cout << "End message -------------------------------------------------------" << endl;

        i = new_msg_start_pos;
        msg_start_pos = new_msg_start_pos;
        // cout << "i:       " << i << endl;
        // cout << "end_pos: " << end_pos << endl;
    }

    // for (auto const &message : messages)
    //    cout << message << "\n";

    p.set_value(move(messages));
}

int main(int argc, char **argv) {
    vector<string> args(argv + 1, argv + argc);

    string input_file;
    tm start_t;
    tm end_t;
    unordered_map<int, string> tags_to_check;

    if (argc < 5) {
        cout << "Invalid input. Should be more than 5 parameters. Use -h to check the correct format." << '\n';
        return -1;
    }

    for (auto i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
            cout << "Help: -fix_dump 9120_in.211215090640911359.fix_dump -time 20211215-20:10:16/20211215-20:10:51 [-tag 35=0 -tag 9=72 ...]" << endl;
            cout << "Program parses input fix_dump file filtering messages by time interval and tags. Output appropriate messages." << endl;
            return 0;
        } else if (*i == "-fix_dump") {
            input_file = *++i;

            if(input_file.empty()) {
                cout << "No input file. Please, input all of main parameters: input_file, time_range. Use -h to check the correct format." << endl;
                return -1;
            }
        } else if (*i == "-time") {
            string time_range = *++i;
            int error_code = 0;
            string error_msg;
            tie (error_code, error_msg, start_t, end_t) = get_start_end_times(time_range);
            if(error_code != 0) {
                cout << error_msg << " Use -h to check the correct format." << endl;
                return -1;
            }
        } else if (*i == "-tag") {
            string tag_value = *++i;
            const auto& [tag, value] = get_tag_value(tag_value);
            tags_to_check[tag] = value;
        }
    }

    cout << "Conditions for filtering:" << endl;
    cout << "Start time: " << put_time(&start_t, "%c") << endl;
    cout << "End time:   " << put_time(&end_t, "%c") << endl;
    for (auto const& [tag, value] : tags_to_check)
        cout << tag << '=' << value << endl;

    ifstream is(input_file, ifstream::binary);
    if (not is) {
        cout << "Can not read file: " << input_file << endl;
        return -1;
    }

    is.seekg (0, is.end);
    auto file_length = is.tellg();
    is.seekg (0, is.beg);

    vector<char> bytes(file_length);
    is.read(bytes.data(), file_length);
    is.close();

    string file_content(bytes.data(), file_length);

    int threads_count = thread::hardware_concurrency() ? thread::hardware_concurrency() : 8;
    // cout << "threads_count: " << threads_count << endl;

    int size_to_process = file_length / threads_count;

    // cout << "length:          " << file_length << endl;
    // cout << "size_to_process: " << size_to_process << endl;

    vector<thread> workers;
    vector<promise<list<string>>> promises(threads_count);
    vector<future<list<string>>> futures;
    for (auto &p : promises) {
        futures.push_back(p.get_future());
    }

    size_t start_pos = 0;
    for(int i = 0; i < threads_count; ++i) {
        auto new_msg_start_pos = file_content.find("8=FIX.4", start_pos + size_to_process);

        if(new_msg_start_pos == string::npos)
            new_msg_start_pos = file_content.size() - 1;

        // cout << "start_pos        : " << start_pos << endl;
        // cout << "new_msg_start_pos: " << new_msg_start_pos << endl << endl;
        workers.push_back(thread(filter_messages, move(promises[i]), ref(file_content), start_pos, new_msg_start_pos, start_t, end_t, cref(tags_to_check)));
        start_pos = new_msg_start_pos;
    }

    try
    {
        for (auto &worker: workers)
            if (worker.joinable())
                worker.join();

        // int i = 0;
        for (auto &fut : futures) {
            auto messages = fut.get();
            // cout << "From future: " << i++ << endl;
            for (auto const &message : messages) {
                cout << message << endl;;
            }
        }
    } catch (future_error const & e) {
        cout << "Future error: " << e.what() << " / " << e.code() << endl;
    } catch (exception const & e) {
        cout << "Standard exception: " << e.what() << endl;
    } catch (...) {
        cout << "Unknown exception." << endl;
    }


    return 1;
}

/* For debug
void filter_messages2(promise<list<string>> && p, const string & file_content) {
    list<string> ll;

    ll.push_back("123");
    ll.push_back("456");

    p.set_value(move(ll));
}
int test()
{
    promise<int> pr;
    auto fut = pr.get_future();

    {
        promise<int> pr2(move(pr));
        pr2.set_value(10);
    }

    return fut.get();
}


void func(promise<list<string>> && p) {
    list<string> ll;

    ll.push_back("123");
    ll.push_back("456");

    p.set_value(move(ll));
}
*/
