import { Chart, ChartConfiguration, Point, registerables } from 'chart.js';

Chart.register(...registerables);

const MAX_ITER = 25;
const COLORS = [
  'Red',
  'Blue',
  'Green',
  'Purple',
  'Black',
  'Orange',
  'Yellow',
  'Brown',
];

const chartTypes = [
  'totalIntraClusterScatter',
  // 'totalInterClusterScatter',
  // 'combined',
];

interface ChartConfig {
  title: string;
}

const chartConfig = ({ title }: ChartConfig): ChartConfiguration => ({
  type: 'scatter',
  data: {
    datasets: [],
  },
  options: {
    elements: {
      point: {
        radius: 5,
        borderWidth: 0,
      },
    },
    animation: {
      duration: 0,
    },
    scales: {
      y: {
        display: false,
      },
      x: {
        display: false,
      },
    },
    plugins: {
      title: {
        display: true,
        text: title,
        font: {
          size: 40,
        },
      },
    },
  },
});

class KMeans {
  private data: Point[] = [];
  private centroids: Point[] = [];
  private clusters: Point[][] = [];
  private prevClusterLengths: number[] = [];
  private chart?: Chart;

  private norm(point: Point) {
    return Math.sqrt(point.x ** 2 + point.y ** 2);
  }

  private mean(data: Point[]) {
    const sum = data.reduce((prev, curr) => ({
      x: prev.x + curr.x,
      y: prev.y + curr.y,
    }));
    return { x: sum.x / data.length, y: sum.y / data.length };
  }

  private distance(p1: Point, p2: Point) {
    return this.norm({ x: p1.x - p2.x, y: p1.y - p2.y });
  }

  private initCentroids(k: number) {
    const chosen = new Set();
    this.centroids = new Array(k);

    for (let i = 0; i < k; i++) {
      let index = Math.floor(Math.random() * this.data.length);

      while (chosen.has(index)) {
        index = Math.floor(Math.random() * this.data.length);
      }

      this.centroids[i] = this.data[index];
      chosen.add(index);
    }
  }

  private intraClusterScatter(index: number) {
    return this.clusters[index]
      .map((point) => Math.pow(this.distance(point, this.centroids[index]), 2))
      .reduce((prev, curr) => prev + curr);
  }

  totalIntraClusterScatter() {
    return this.clusters
      .map((_, index) => this.intraClusterScatter(index))
      .reduce((prev, curr) => prev + curr);
  }

  private updateClusters() {
    this.clusters = new Array(this.centroids.length).fill(null).map((_) => []);

    this.data.forEach((point) => {
      const distances = this.centroids
        .map((centroid, index) => ({
          index,
          dist: this.distance(centroid, point),
        }))
        .sort((a, b) => a.dist - b.dist);

      this.clusters[distances[0].index].push(point);
    });

    this.clusters.forEach((cluster, index) =>
      cluster.push(this.centroids[index])
    );
  }

  private updateCentroids() {
    this.centroids = this.centroids.map((_, index) =>
      this.mean(this.clusters[index])
    );
  }

  private updateChart() {
    if (this.chart) {
      this.chart.data.datasets = this.clusters.map((cluster, index) => ({
        label: `Cluster ${index}`,
        data: cluster,
        pointBackgroundColor: COLORS[index],
      }));
      this.chart.update();
    }
  }

  private reachedLocalMaxima() {
    return this.clusters.every(
      (cluster, index) => this.prevClusterLengths[index] === cluster.length
    );
  }

  private restart(clusters: number) {
    this.initCentroids(clusters);
    this.updateClusters();
  }

  constructor(data?: string, chart?: Chart) {
    if (data) {
      try {
        this.chart = chart;

        this.data = data
          .split('\n')
          .map((line) =>
            line
              .trim()
              .split(/\s+/)
              .map((_) => Number(_))
          )
          .map(([x, y]) => ({ x, y }));
      } catch (e) {
        console.error(e);
      }
    }
  }

  async clusterize(clusters: number, evaluation: any) {
    let restarts = 20;
    let bestSoFar: number | null = null;

    while (restarts-- > 0) {
      this.restart(clusters);

      let iter = MAX_ITER;

      while (iter-- > 0) {
        console.log(`Iteration: ${MAX_ITER - iter}`);

        this.updateCentroids();
        this.updateClusters();

        if (this.reachedLocalMaxima()) {
          console.log('Reached local maxima');
          break;
        }

        this.prevClusterLengths = this.clusters.map(
          (cluster) => cluster.length
        );
      }

      const current = evaluation();

      if (!bestSoFar || bestSoFar > current) {
        bestSoFar = current;
        this.updateChart();
      }

      console.log(`Current: ${current} | Best: ${bestSoFar}`);
    }
  }
}

let kMeans = new KMeans();

document.getElementById('file')?.addEventListener('change', (event: any) => {
  document.getElementById('fileName')!.innerHTML =
    event.target.files[0]?.name ?? 'No dataset';
});

document.getElementById('data')?.addEventListener('submit', (event: any) => {
  event.preventDefault();

  const file: File | undefined = (
    document.getElementById('file') as HTMLInputElement
  ).files?.[0];

  if (!file) {
    throw 'missing dataset';
  }

  const clusters = Number(
    (document.getElementById('clusters') as HTMLInputElement).value
  );

  let reader = new FileReader();
  reader.onload = async () => {
    chartTypes.forEach(async (type) => {
      const chart = new Chart(
        document.getElementById(`chart-${type}`) as HTMLCanvasElement,
        chartConfig({ title: `${file.name.split('.')[0]}-${type}` })
      );

      kMeans = new KMeans(reader.result as string, chart);
      await kMeans.clusterize(
        clusters,
        kMeans[type as keyof KMeans].bind(kMeans)
      );
    });
  };
  reader.readAsText(file, 'utf8');
});
