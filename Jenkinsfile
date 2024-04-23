pipeline {
  agent any
  stages {
    stage('stage1') {
      steps {
        sh 'echo "Hello world!"'
        mail(subject: 'Hello', body: 'Body', from: 'huaguanma@tencent.com', to: 'huaguanma@tencent.com')
      }
    }

    stage('stage2') {
      steps {
        sh 'echo "Hello world!"'
      }
    }

    stage('error') {
      steps {
        sleep 3
      }
    }

  }
}