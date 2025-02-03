import androidx.compose.desktop.ui.tooling.preview.Preview
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.TextFieldValue
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.draw.clip
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.net.HttpURLConnection
import java.net.URL
import kotlinx.serialization.*
import kotlinx.serialization.json.*
import androidx.compose.ui.window.Window
import androidx.compose.ui.window.application
import java.net.Socket

@Serializable
data class ChatResponse(val title: String, val response: String)

@Composable
@Preview
fun ChatUI() {
    var inputText by remember { mutableStateOf(TextFieldValue()) }
    val chatHistory = remember { mutableStateListOf<Pair<String, Boolean>>() }
    val coroutineScope = rememberCoroutineScope()
    var isDarkMode by remember { mutableStateOf(false) }

    val backgroundColor = if (isDarkMode) Color(0xFF121212) else Color(0xFFF5F5F5)
    val textColor = if (isDarkMode) Color.White else Color.Black
    val userBubbleColor = if (isDarkMode) Color(0xFF1E88E5) else Color(0xFF90CAF9)
    val botBubbleColor = if (isDarkMode) Color(0xFF37474F) else Color(0xFFCFD8DC)

    Column(
        modifier = Modifier.fillMaxSize().background(backgroundColor).padding(16.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text("BEC-MM ChatBot", fontSize = 24.sp, color = textColor)
            Row {
                Button(onClick = { chatHistory.add("Help: Type a question based on the modules and click Send." to false) }) {
                    Text("Help")
                }
                Spacer(modifier = Modifier.width(8.dp))
                Button(onClick = { 
                    chatHistory.add(
                        """
                        Faculty of Automatic Control and Computer Science
                        
                        "Gender, Digitalization, Green: Ensuring a Sustainable Future for all in Europe" 
                        Ref. Project: 2023-1-RO01- KA220-HED-000154433
                        
                        Partnership:
                        - Universitatea de Stiinte Agricole si Medicina Veterinara, Bucuresti, Romania
                        - Universitatea Nationala de Stiinta si Tehnologie POLITEHNICA București, Romania
                        - Universitat Autonoma de Barcelona, Espana
                        - Universidade do Porto, República Portuguesa
                        - Uzhgorodskyi Nacionalnyi Universitet, Ukraina
                        
                        Course Coordinators: Mihai Caramihai, Daniel Chis.
                        
                        Students:
                        - Stefan Bucur  -  stefan.bucur0707@stud.acs.upb.ro
                        - Costin Grasu  -  costin.grasu@stud.acs.upb.ro
                        - Mario Timoc  -  ovidiu_mario.timoc@stud.acs.upb.ro
                        - Miruna Tancu  -  miruna.tancu@stud.acs.upb.ro
                        - Eduard Simion  -  eduard.simion2410@stud.acs.upb.ro
                        """.trimIndent() to false
                    ) 
                }) {
                    Text("Credentials")
                }
                
                Spacer(modifier = Modifier.width(8.dp))
                Switch(checked = isDarkMode, onCheckedChange = { isDarkMode = it })
            }
        }

        Column(
            modifier = Modifier.weight(1f).fillMaxWidth().verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.Bottom
        ) {
            chatHistory.forEach { (message, isUser) ->
                ChatBubble(message, isUser, userBubbleColor, botBubbleColor, textColor)
            }
        }

        Row(
            modifier = Modifier.fillMaxWidth().padding(top = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            BasicTextField(
                value = inputText,
                onValueChange = { inputText = it },
                keyboardOptions = KeyboardOptions.Default.copy(imeAction = ImeAction.Send),
                keyboardActions = KeyboardActions(onSend = {
                    val userMessage = inputText.text.trim()
                    if (userMessage.isNotEmpty()) {
                        chatHistory.add(userMessage to true)
                        inputText = TextFieldValue()
                        coroutineScope.launch { fetchChatResponse(userMessage, chatHistory) }
                    }
                }),
                modifier = Modifier.weight(1f).background(Color.White, RoundedCornerShape(20.dp)).padding(12.dp)
            )
            Spacer(modifier = Modifier.width(8.dp))
            Button(onClick = {
                val userMessage = inputText.text.trim()
                if (userMessage.isNotEmpty()) {
                    chatHistory.add(userMessage to true)
                    inputText = TextFieldValue()
                    coroutineScope.launch { fetchChatResponse(userMessage, chatHistory) }
                }
            }, shape = RoundedCornerShape(20.dp)) {
                Text("Send")
            }
        }
    }
}

@Composable
fun ChatBubble(message: String, isUser: Boolean, userBubbleColor: Color, botBubbleColor: Color, textColor: Color) {
    Box(
        modifier = Modifier.fillMaxWidth().padding(4.dp),
        contentAlignment = if (isUser) Alignment.CenterEnd else Alignment.CenterStart
    ) {
        Box(
            modifier = Modifier.clip(RoundedCornerShape(20.dp)).background(if (isUser) userBubbleColor else botBubbleColor).padding(12.dp)
        ) {
            Text(text = message, color = textColor, fontSize = 16.sp)
        }
    }
}

suspend fun fetchChatResponse(userMessage: String, chatHistory: MutableList<Pair<String, Boolean>>) {
    val url = "http://127.0.0.1:5000/chat"
    val jsonBody = Json.encodeToString(mapOf("question" to userMessage))

    try {
        val responseText = withContext(Dispatchers.IO) {
            val connection = URL(url).openConnection() as HttpURLConnection
            connection.requestMethod = "POST"
            connection.doOutput = true
            connection.setRequestProperty("Content-Type", "application/json")

            connection.outputStream.use { it.write(jsonBody.toByteArray()) }
            connection.inputStream.bufferedReader().use { it.readText() }
        }

        val chatResponse = Json.decodeFromString<ChatResponse>(responseText)

        withContext(Dispatchers.Main) {
            chatHistory.add(chatResponse.title to false)
            chatHistory.add(chatResponse.response to false)
        }
    } catch (e: Exception) {
        e.printStackTrace()
        withContext(Dispatchers.Main) {
            chatHistory.add("❌ Error: Could not connect to Flask server. Make sure `main.exe` is running!" to false)
        }
    }
}

fun isFlaskRunning(): Boolean {
    return try {
        Socket("127.0.0.1", 5000).use { true }
    } catch (e: Exception) {
        false
    }
}

fun main() {
    if (!isFlaskRunning()) {
        println("❌ Flask server is not running. Please start `main.exe` first!")
        return
    }

    application {
        Window(onCloseRequest = ::exitApplication, title = "BEC-MM ChatBot") {
            ChatUI()
        }
    }
}
