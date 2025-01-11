#include <curl/curl.h>
#include <iostream>
#include <string>
#include <sstream>

// Callback function to write CURL response
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    if (userp == nullptr || contents == nullptr) {
        return 0; // Return 0 if userp or contents is null
    }
    try {
        userp->append(static_cast<char*>(contents), size * nmemb);
        return size * nmemb;
    } catch (...) {
        return 0; // Handle any exceptions gracefully
    }
}

// Helper function to send POST requests to Flask server
std::string sendPostRequest(const std::string& url, const std::string& message) {
    CURL* curl = curl_easy_init(); // Initialize CURL
    if (!curl) {
        return "Error initializing CURL.";
    }

    std::string response;
    struct curl_slist* headers = nullptr;

    try {
        // Add headers for JSON content type
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!headers) {
            throw std::runtime_error("Error creating headers.");
        }

        // Format the JSON payload
        std::ostringstream oss;
        oss << "{\"message\": \"" << message << "\"}";
        std::string payload = oss.str(); // Ensure the payload is a valid string

        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str()); // Use a valid string for payload
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response = "Error: " + std::string(curl_easy_strerror(res));
        }
    } catch (const std::exception& e) {
        response = std::string("Exception: ") + e.what();
    } catch (...) {
        response = "Exception: Unknown error.";
    }

    // Cleanup
    if (headers) {
        curl_slist_free_all(headers); // Free the headers list
    }
    curl_easy_cleanup(curl); // Cleanup CURL resources

    if (response.empty()) {
        response = "Error: Empty response from server.";
    }

    return response;
}

int main() {
    std::string server_url = "http://192.168.1.5:5000/chat"; // Flask server URL
    std::string user_input;

    std::cout << "Chat with the bot! Type 'exit' to quit.\n";

    while (true) {
        // Get user input
        std::cout << "You: ";
        std::getline(std::cin, user_input);
        if (user_input == "exit") {
            std::cout << "Goodbye!\n";
            break;
        }

        // Send the input to the Flask server and get the response
        std::string bot_response = sendPostRequest(server_url, user_input);

        // Print the response
        std::cout << "Bot: " << bot_response << "\n";
    }

    return 0;
}
