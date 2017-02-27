package model;

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Port;
import edu.cmu.sphinx.api.Configuration;
import edu.cmu.sphinx.api.LiveSpeechRecognizer;
import edu.cmu.sphinx.api.SpeechResult;
import java.util.Scanner;

public class Main {

    // Logger
    private Logger logger = Logger.getLogger(getClass().getName());

    // Variables
    private String result;

    // Threads
    Thread speechThread;
    Thread resourcesThread;

    // LiveRecognizer
    private LiveSpeechRecognizer recognizer;

	private Scanner a;

    /**
     * Constructor
     */
    int language = 0;
    
    public Main() throws IOException {

        // Loading Message
        logger.log(Level.INFO, "Loading..\n");

        // Choix de langue du jeu
        int securite = 0;
        do {
            System.out.println("Choose your language: 1-English 2-French");
            a = new Scanner(System.in);
            language = a.nextInt();
            if ((language == 1) || (language == 2)) {
                securite = 1;
            }
        } while (securite == 0);

        // Configuration
        Configuration configuration = new Configuration();

        // Load model from the jar
        if (language == 1) {
            configuration.setAcousticModelPath("resource:/edu/cmu/sphinx/models/en-us/en-us");
            configuration.setDictionaryPath("resource:/edu/cmu/sphinx/models/en-us/cmudict-en-us.dict");
            configuration.setGrammarPath("resource:/grammars");
            configuration.setGrammarName("grammar");
            configuration.setUseGrammar(true);
        } else {
            configuration.setAcousticModelPath("resource:/edu/cmu/sphinx/models/fr-fr/fr-fr");
            configuration.setDictionaryPath("resource:/edu/cmu/sphinx/models/fr-fr/frenchwords62k.dict");
            configuration.setGrammarPath("resource:/grammars_fr");
            configuration.setGrammarName("grammar");
            configuration.setUseGrammar(true);
        }

        try {
            recognizer = new LiveSpeechRecognizer(configuration);
        } catch (IOException ex) {
            logger.log(Level.SEVERE, null, ex);
        }

        // Start recognition process pruning previously cached data.
        recognizer.startRecognition(true);

        // Start the Thread
        startSpeechThread();
        startResourcesThread();
    }

    /**
     * Starting the main Thread of speech recognition
     */
    protected void startSpeechThread() {

        // alive?
        if (speechThread != null && speechThread.isAlive()) {
            return;
        }

        // initialise
        speechThread = new Thread(() -> {
            try {

                while (true) {
                    /*
                     * This method will return when the end of speech is
                     * reached. Note that the end pointer will determine the end
                     * of speech.
                     */
                    logger.log(Level.INFO, "Waiting...\n");
                    Scanner number = new Scanner(System.in); // Attente d'une réponse du programme mécanisme
                    int saisie = number.nextInt();
                    logger.log(Level.INFO, "You can start to speak...\n");
                    SpeechResult speechResult = recognizer.getResult();

                    if (speechResult != null) {
                        result = speechResult.getHypothesis();
                        
                        if (language == 1) {
                            result = speechResult.getHypothesis().replaceAll("one", "1").replaceAll("two", "2").replaceAll("three", "3").replaceAll("four", "4").replaceAll("five", "5").replaceAll("six", "6").replaceAll("seven", "7").replaceAll("eight", "8");
                        } else {
                            result = speechResult.getHypothesis().replaceAll("un", "1").replaceAll("deux", "2").replaceAll("trois", "3").replaceAll("quatre", "4").replaceAll("cinq", "5").replaceAll("six", "6").replaceAll("sept", "7").replaceAll("huit", "8");
                        }

                        System.out.println(result);

                    } else {
                        logger.log(Level.INFO, "Error\n");
                    }

                }
            } catch (Exception ex) {
                logger.log(Level.WARNING, null, ex);
            }

            logger.log(Level.INFO, "SpeechThread has exited...");
        }
        );

        // Start
        speechThread.start();

    }

    /**
     * Starting a Thread that checks if the resources needed to the
     * SpeechRecognition library are available
     */
    protected void startResourcesThread() {

        // alive?
        if (resourcesThread != null && resourcesThread.isAlive()) {
            return;
        }

        resourcesThread = new Thread(() -> {
            try {

                // Detect if the microphone is available
                while (true) {
                    if (AudioSystem.isLineSupported(Port.Info.MICROPHONE)) {
                        // logger.log(Level.INFO, "Microphone is available.\n")
                    } else {
                        // logger.log(Level.INFO, "Microphone is not
                        // available.\n")

                    }

                    // Sleep some period
                    Thread.sleep(350);
                }

            } catch (InterruptedException ex) {
                logger.log(Level.WARNING, null, ex);
                resourcesThread.interrupt();
            }
        });

        // Start
        resourcesThread.start();
    }

    /**
     * Takes a decision based on the given result
     */
    public void makeDesicion(String result) {
        //implemented in the part 2
    }

    /**
     * Java Main Application Method
     *
     * @param args
     */
    public static void main(String[] args) throws IOException {

        // // Be sure that the user can't start this application by not giving
        // the
        // // correct entry string
        // if (args.length == 1 && "SPEECH".equalsIgnoreCase(args[0]))
        new Main();
        // else
        // Logger.getLogger(Main.class.getName()).log(Level.WARNING, "Give me
        // the correct entry string..");

    }

}
