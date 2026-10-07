let nomeAluno = "";


        function entrarSistema() {

            const usuario =
                document
                    .getElementById("usuario")
                    .value
                    .trim();


            const mensagem =
                document
                    .getElementById("loginMessage");


            if (usuario === "") {

                mensagem.style.color = "#ff5577";

                mensagem.textContent =
                    "✕ Digite seu nome para entrar.";

                return;
            }


            nomeAluno = usuario;


            mensagem.style.color = "#00ff88";

            mensagem.textContent =
                "✓ Bem-vindo, " + nomeAluno + "!";


            setTimeout(function () {

                document
                    .getElementById("loginScreen")
                    .style.display = "none";


                document
                    .getElementById("site")
                    .style.display = "block";


                window.scrollTo({
                    top: 0,
                    behavior: "instant"
                });


                iniciarAnimacoes();

            }, 700);

        }

        function criarConta() {

            const nome =
                document
                    .getElementById("novoUsuario")
                    .value
                    .trim();


            const mensagem =
                document
                    .getElementById("cadastroMessage");


            if (nome === "") {

                mensagem.style.color = "#ff5577";

                mensagem.textContent =
                    "✕ Digite seu nome.";

                return;
            }


            nomeAluno = nome;


            mensagem.style.color = "#00ff88";

            mensagem.textContent =
                "✓ Conta criada com sucesso!";


            setTimeout(function () {

                document
                    .getElementById("usuario")
                    .value = nomeAluno;


                voltarLogin();

            }, 700);

        }


        function mostrarCadastro() {

            document
                .getElementById("loginPage")
                .style.display = "none";


            document
                .getElementById("cadastroPage")
                .style.display = "block";

        }


        function voltarLogin() {

            document
                .getElementById("cadastroPage")
                .style.display = "none";


            document
                .getElementById("loginPage")
                .style.display = "block";


            document
                .getElementById("cadastroMessage")
                .textContent = "";

        }

        function iniciarAnimacoes() {

            const elementos =
                document.querySelectorAll(".reveal");


            const observer =
                new IntersectionObserver(

                    (entries) => {

                        entries.forEach(
                            (entry) => {

                                if (entry.isIntersecting) {

                                    entry.target
                                        .classList
                                        .add("active");

                                }

                            }
                        );

                    },

                    {
                        threshold: 0.12
                    }

                );


            elementos.forEach(
                (elemento) => {

                    observer.observe(elemento);

                }
            );

        }

        window.addEventListener(
            "scroll",
            function () {

                if (
                    document
                        .getElementById("site")
                        .style.display !== "block"
                ) {
                    return;
                }


                const scroll =
                    window.scrollY;


                const altura =
                    document.documentElement.scrollHeight -
                    document.documentElement.clientHeight;


                const porcentagem =
                    altura > 0
                        ? (scroll / altura) * 100
                        : 0;


                document
                    .getElementById("progress")
                    .style.width =
                    porcentagem + "%";


                const top =
                    document.getElementById("top");


                if (scroll > 500) {

                    top.classList.add("show");

                } else {

                    top.classList.remove("show");

                }

            }
        );


        function voltarTopo() {

            window.scrollTo({

                top: 0,

                behavior: "smooth"

            });

        }

        function ativarSensor() {

            const led =
                document.getElementById("statusLed");


            const texto =
                document.getElementById("statusText");


            const valor =
                document.getElementById("sensorValue");


            const barra =
                document.getElementById("sensorBar");


            led.classList.add("on");


            texto.textContent =
                "Sensor detectando presença...";


            const numero =
                Math.floor(
                    Math.random() * 35
                ) + 65;


            valor.textContent =
                numero + "%";


            barra.style.width =
                numero + "%";

        }


        function acionarServo() {

            const servo =
                document.getElementById("servoValue");


            const barra =
                document.getElementById("servoBar");


            const led =
                document.getElementById("statusLed");


            const texto =
                document.getElementById("statusText");


            servo.textContent =
                "90°";


            barra.style.width =
                "50%";


            led.classList.add("on");


            texto.textContent =
                "Servo motor acionado!";

        }


        function resetarSistema() {

            document
                .getElementById("statusLed")
                .classList
                .remove("on");


            document
                .getElementById("statusText")
                .textContent =
                "Sistema aguardando...";


            document
                .getElementById("sensorValue")
                .textContent =
                "65%";


            document
                .getElementById("sensorBar")
                .style.width =
                "65%";


            document
                .getElementById("servoValue")
                .textContent =
                "0°";


            document
                .getElementById("servoBar")
                .style.width =
                "0%";

        }

        let imagemAtual = 0;


        const totalImagens = 6;


        function atualizarGaleria() {

            const track =
                document.getElementById(
                    "galleryTrack"
                );


            const dots =
                document.querySelectorAll(
                    ".gallery-dot"
                );


            const thumbnails =
                document.querySelectorAll(
                    ".gallery-thumbnail"
                );


            const counter =
                document.getElementById(
                    "galleryCounter"
                );

            track.style.transform =
                "translateX(-" +
                (imagemAtual * 100) +
                "%)";

            dots.forEach(
                (dot, index) => {

                    dot.classList.toggle(
                        "active",
                        index === imagemAtual
                    );

                }
            );

            thumbnails.forEach(
                (thumbnail, index) => {

                    thumbnail.classList.toggle(
                        "active",
                        index === imagemAtual
                    );

                }
            );

            counter.textContent =
                (imagemAtual + 1) +
                " / " +
                totalImagens;

        }


        function mudarImagem(direcao) {

            imagemAtual += direcao;


            if (imagemAtual >= totalImagens) {

                imagemAtual = 0;

            }

            if (imagemAtual < 0) {

                imagemAtual =
                    totalImagens - 1;

            }


            atualizarGaleria();

        }


        function irParaImagem(indice) {

            if (
                indice < 0 ||
                indice >= totalImagens
            ) {
                return;
            }


            imagemAtual = indice;


            atualizarGaleria();

        }


        document.addEventListener(
            "keydown",
            function (event) {

                const site =
                    document.getElementById("site");


                if (
                    site.style.display !== "block"
                ) {
                    return;
                }

                if (event.key === "ArrowLeft") {

                    mudarImagem(-1);

                }

                if (event.key === "ArrowRight") {

                    mudarImagem(1);

                }

            }
        );

        let toqueInicial = 0;


        const galeria =
            document.querySelector(
                ".gallery-carousel"
            );


        galeria.addEventListener(
            "touchstart",
            function (event) {

                toqueInicial =
                    event.touches[0].clientX;

            },
            {
                passive: true
            }
        );


        galeria.addEventListener(
            "touchend",
            function (event) {

                const toqueFinal =
                    event.changedTouches[0].clientX;


                const distancia =
                    toqueFinal - toqueInicial;


                if (Math.abs(distancia) < 50) {
                    return;
                }


                if (distancia < 0) {

                    mudarImagem(1);

                } else {

                    mudarImagem(-1);

                }

            },
            {
                passive: true
            }
        );

        function abrirVideo() {

            alert(
                "Aqui você pode colocar o vídeo da apresentação do projeto."
            );

        }


        atualizarGaleria();


        console.log(
            "🤖 Escola Automática iniciada!"
        );